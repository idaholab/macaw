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
  params.addParam<bool>("verbose", false, "Whether to print collision information");

  return params;
}

CollisionKernel::CollisionKernel(const InputParameters & params)
  : IntegralRayKernelBase(params),
    _T(coupledValue("temperature")),
    _verbose(getParam<bool>("verbose"))
{
  // Check that the temperature variable is a constant monomial
  // TODO check properly
  if (getFieldVar("temperature", 0)->order() != 1)
    paramError("temperature", "Only CONST MONOMIAL temperatures are currently supported.");

  // Build map from subdomains to OpenMC materials
  const auto blocks = getParam<std::vector<unsigned int>>("blocks");
  const auto materials = getParam<std::vector<unsigned int>>("materials");
  for (unsigned int i = 0; i < blocks.size(); i++)
    _block_to_openmc_materials.insert(std::make_pair<int, int>(blocks[i], materials[i]));

  // Check that all materials do exist in OpenMC, otherwise it will crash at XS calculation


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
}

void
CollisionKernel::onSegment()
{
  // Use a fake neutron to compute the cross sections
  auto p = &_particles[_tid];

  // resize to account for filters specified in moose
  p->filter_matches().resize(openmc::model::tally_filters.size());
  p->sqrtkT() = std::sqrt(openmc::K_BOLTZMANN * _T[0]);
  p->material() = _block_to_openmc_materials.at(_current_elem->subdomain_id());
  p->coord(p->n_coord() - 1).cell = 0; // avoids a geometry search
  p->u() = {
      currentRay()->direction()(0), currentRay()->direction()(1), currentRay()->direction()(2)};
  p->E() = currentRay()->auxData(0);
  p->wgt() = currentRay()->auxData(1);
  p->n_progeny() = currentRay()->auxData(2);

  // Set the particle id at the right location for setting n_progeny in event_death
  p->id() = currentRay()->auxData(3);

  // Reset the OpenMC particle status
  p->alive() = true;

  p->event_calculate_xs();

  // Compute distance to next collision
  // p.event_advance();
  // TODO scores tracklength tallies as well
  // TODO Can we use this??

  const Real collision_distance = -std::log(openmc::prn(p->current_seed())) / p->macro_xs().total;

  // Contribute to the tracklength keff estimator
  p->keff_tally_tracklength() += p->wgt() *
      std::min(collision_distance, _current_segment_length) * p->macro_xs().nu_fission;

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

    // Compute collision
    p->event_collide();

    if (_verbose)
      _console << "Collision event " << int(p->event()) << " Energy " << currentRay()->auxData(0)
                << " -> " << p->E() << " block " << _current_elem->subdomain_id() << " material "
                << p->material() << std::endl;

    // Update Ray direction
    Point new_direction(p->u()[0], p->u()[1], p->u()[2]);
    if (p->event() == openmc::TallyEvent::SCATTER)
      changeRayStartDirection(current_position, new_direction);

    // Update Ray energy
    currentRay()->auxData(0) = p->E();

    // Keep track of weight (for implicit capture)
    currentRay()->auxData(1) = p->wgt();

    // Keep track of particle number of progeny
    currentRay()->auxData(2) = p->n_progeny();

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

    // Handle secondary particles immediately
    // p->alive() = false;
    // p->event_revive_from_secondary();
    // if (p->n_event() == 0)
    // {
    //   if (_verbose)
    //     _console << "Sampled secondary particle, creating new ray " << p->r() << " " << p->E()
    //     << "eV " << p->u() << std::endl;
    //
    //   // Create a new Ray with starting information
    //   Point start(p->r()[0], p->r()[1], p->r()[2]);
    //   Point direction(p->u()[0], p->u()[1], p->u()[2]);
    //   std::shared_ptr<Ray> ray = acquireRay(start, direction);
    //
    //   // Store neutron information
    //   ray->auxData(0) = p->E();
    //   ray->auxData(1) = p->wgt();
    //
    //   // Reset number of progeny particles
    //   ray->auxData(2) = 0;
    //
    //   // Keep track of openmc particle id
    //   ray->auxData(3) = p->id();
    //
    //   moveRayToBuffer(ray);
    // }
  }
}
