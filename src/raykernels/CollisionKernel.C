//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CollisionKernel.h"

// openmc includes
#include "openmc/capi.h"
#include "openmc/constants.h"
#include "openmc/material.h"
#include "openmc/particle.h"
#include "openmc/particle_data.h"
#include "openmc/random_lcg.h"
#include "openmc/simulation.h"
#include "openmc/tallies/tally.h"

registerMooseObject("MaCawApp", CollisionKernel);

InputParameters
CollisionKernel::validParams()
{
  InputParameters params = IntegralRayKernelBase::validParams();

  params.addClassDescription("Computes distance to the next collision. "
                             "Samples the reaction. Computes trajectory changes. "
                             "Contributes to tallies.");

  params.addRequiredCoupledVar("temperature", "The temperature of the medium.");
  params.addRequiredParam<std::vector<unsigned int>>("blocks",
                                                     "Blocks on which this kernel is defined.");
  params.addRequiredParam<std::vector<unsigned int>>("materials",
                                                     "OpenMC material ids for each block.");
  params.addParam<Real>("z_coord", 0, "The axial coordinate for 2D calculations");
  params.addParam<bool>("verbose", false, "Whether to print collision information");

  return params;
}

CollisionKernel::CollisionKernel(const InputParameters & params)
  : IntegralRayKernelBase(params),
    _T(coupledValue("temperature")),
    _is_2D(_mesh.dimension() == 2),
    _z_coord(getParam<Real>("z_coord")),
    _verbose(getParam<bool>("verbose"))
{
  // Check that the temperature variable is a constant monomial
  // TODO check properly
  if (getFieldVar("temperature", 0)->order() != 1)
    paramError("temperature", "Only CONST MONOMIAL temperatures are currently supported.");

  // Check for 2D parameters
  if (_is_2D && !params.isParamSetByUser("z_coord"))
    mooseError("The z_coord parameter must be specified for 2D calculations");

  // Build map from subdomains to OpenMC materials
  const auto blocks = getParam<std::vector<unsigned int>>("blocks");
  const auto materials = getParam<std::vector<unsigned int>>("materials");
  if (blocks.size() != materials.size())
    paramError("blocks", "The blocks parameter must be the same size as the materials parameter.");
  for (unsigned int i = 0; i < blocks.size(); i++)
    _block_to_openmc_materials.insert(std::make_pair<int, int>(blocks[i], materials[i]));

  // Resize the neutrons objects used to call OpenMC routines and initialize the seeds
  // TODO optimization: create these neutrons in the study, re-use them everywhere
  _particles.resize(libMesh::n_threads());
  for (unsigned int i = 0; i < _particles.size(); i++)
    openmc::initialize_history(_particles[i], i + 1);
}

void
CollisionKernel::initialSetup()
{
  if (_verbose)
    _console << "Kernel initial setup" << std::endl;

  // Check that all materials do exist in OpenMC, otherwise it will crash at XS calculation
  for (auto & m : _block_to_openmc_materials)
  {
    auto search = openmc::model::material_map.find(m.second);
    if (search == openmc::model::material_map.end())
      mooseError("Could not find material ", m.second, " in OpenMC materials.");
  }

  // Check that all blocks exist, as a sanity check only
  for (auto & b : _block_to_openmc_materials)
    if (!hasBlocks(b.first))
      mooseWarning("Could not find specified block ", b.first, " for the collision kernel.");
}

void
CollisionKernel::onSegment()
{
  // Use a fake neutron to compute the cross sections
  auto p = &_particles[_tid];

  // resize to account for filters specified in moose
  p->filter_matches().resize(openmc::model::tally_filters.size());
  p->sqrtkT() = std::sqrt(openmc::K_BOLTZMANN * _T[0]);
  p->material() = _block_to_openmc_materials.at(_current_elem->subdomain_id()) - 1;
  p->coord(p->n_coord() - 1).universe = _current_subdomain_id;
  p->coord(p->n_coord() - 1).cell = _current_elem->id();
  p->u() = {
      currentRay()->direction()(0), currentRay()->direction()(1), currentRay()->direction()(2)};
  p->E() = currentRay()->auxData(0);
  p->wgt() = currentRay()->auxData(1);
  p->n_progeny() = currentRay()->auxData(2);

  // Set the particle id at the right value for setting n_progeny in event_death
  p->id() = currentRay()->auxData(3);

  // To avoid an overflow in the n_progeny array for neutrons which would have
  // changed domain, we adjust the value, however this prevents us from using the
  // openmc sorting algorithm
  if (p->id() - 1 - openmc::simulation::work_index[comm().rank()] >=
      openmc::simulation::work_per_rank)
    p->id() = 1 + openmc::simulation::work_index[comm().rank()];
  else if (p->id() - 1 - openmc::simulation::work_index[comm().rank()] < 0)
    p->id() = 1 + openmc::simulation::work_index[comm().rank()];

  // Set the particle seed for consistent random number generation
  p->seeds(0) = currentRay()->auxData(4);
  p->seeds(1) = currentRay()->auxData(5);
  p->seeds(2) = currentRay()->auxData(6);

  // Set particle type
  p->type() = openmc::ParticleType(currentRay()->auxData(7));

  // Reset the OpenMC particle status
  p->alive() = true;

  // Compute all cross sections
  p->event_calculate_xs();

  // Compute distance to next collision
  // p.event_advance();
  // TODO scores tracklength tallies as well
  // TODO Can we use this??

  const Real collision_distance = -std::log(openmc::prn(p->current_seed())) / p->macro_xs().total;

  // Contribute to the tracklength keff estimator
  p->keff_tally_tracklength() += p->wgt() *
      std::min(collision_distance, _current_segment_length) * p->macro_xs().nu_fission;

  // Keep track of the particle seed for consistent random number generation
  currentRay()->auxData(4) = p->seeds(0);
  currentRay()->auxData(5) = p->seeds(1);
  currentRay()->auxData(6) = p->seeds(2);

  // Shorter than next intersection, ray tracing will take care of moving the
  // particle
  if (collision_distance > _current_segment_length)
  {
    // p.event_cross_surface();
    // TODO Score track length tallies
    // TODO Score surface tallies
    return;
  }
  else
  {
    // Advance ray (really, move backwards)
    Point current_position =
        currentRay()->currentPoint() -
        (_current_segment_length - collision_distance) * currentRay()->direction();

    // Adapt for two dimensions
    if (_is_2D)
      current_position(2) = _z_coord;

    // Set the particle position to the collision location for banking sites
    p->r() = {current_position(0), current_position(1), current_position(2)};

    // Compute collision
    p->event_collide();

    if (_verbose)
      _console << "Collision event " << int(p->event()) << " Energy " << currentRay()->auxData(0)
               << " -> " << p->E() << " block " << _current_elem->subdomain_id() << " material "
               << p->material() << " progeny " << p->n_progeny() << " ids " << p->id() << " / "
               << currentRay()->id() << std::endl;

    // Scattering direction is stored on neutron
    Point new_direction(p->u()[0], p->u()[1], p->u()[2]);

    // Adapt for two dimensions
    if (_is_2D)
      new_direction(2) = 0;

    if (p->event() == openmc::TallyEvent::SCATTER)
      changeRayStartDirection(current_position, new_direction);

    // Update Ray energy
    currentRay()->auxData(0) = p->E();

    // Keep track of weight (for implicit capture)
    currentRay()->auxData(1) = p->wgt();

    // Keep track of particle number of progeny
    currentRay()->auxData(2) = p->n_progeny();

    // Keep track of the particle seed for consistent random number generation
    currentRay()->auxData(4) = p->seeds(0);
    currentRay()->auxData(5) = p->seeds(1);
    currentRay()->auxData(6) = p->seeds(2);

    // Mark the ray as 'should not continue' if absorption was sampled
    // TODO Handle secondary particles
    // Need to create a new ray as soon as the particles are sampled, as the kernel can only
    // change direction and create rays in the same element
    // TODO Need to track ray ids for progeny when adding new rays
    if (p->event() == openmc::TallyEvent::KILL || p->event() == openmc::TallyEvent::ABSORB)
    {
      currentRay()->setShouldContinue(false);
      p->event_death();
    }

    // Handle secondary particles immediately after they are sampled, as the particle
    // could change domain, and rays cannot be created in a different domain
    p->alive() = false;
    p->event_revive_from_secondary();
    if (false && p->n_event() == 0)
    {
      // Create a new Ray with starting information
      Point start(p->r()[0], p->r()[1], p->r()[2]);
      Point direction(p->u()[0], p->u()[1], p->u()[2]);
      if (_is_2D)
      {
        start(2) = _z_coord;
        direction(2) = 0;
      }
      std::shared_ptr<Ray> ray = acquireRay(start, direction);

      if (_verbose)
        _console << "Sampled secondary particle type " << int(p->type()) << ", creating new ray "
                 << ray->id() << " at " << p->r() << " " << p->E() << "eV " << std::endl;

      // Store neutron information
      ray->auxData(0) = p->E();
      ray->auxData(1) = p->wgt();

      // Reset number of progeny particles
      ray->auxData(2) = 0;

      // Need to generate a new particle id
      // FIXME This is not deterministic
      openmc::Particle p2;
      ray->auxData(3) = p2.id();

      // Keep track of the particle seed for consistent random number generation
      ray->auxData(4) = p->seeds(0);
      ray->auxData(5) = p->seeds(1);
      ray->auxData(6) = p->seeds(2);

      // Keep track of particle type
      ray->auxData(7) = int(p->type());

      moveRayToBuffer(ray);
    }
  }
}
