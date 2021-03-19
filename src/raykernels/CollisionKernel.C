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
#include "openmc/random_lcg.h"

registerMooseObject("MaCawApp", CollisionKernel);

InputParameters
CollisionKernel::validParams()
{
  InputParameters params = IntegralRayKernelBase::validParams();

  params.addClassDescription("Computes distance to the next collision. "
                             "Samples the reaction. Computes trajectory changes. "
                             "Contributes to tallies.");

  params.addRequiredCoupledVar("temperature", "The temperature of the medium.");
  params.addRequiredParam<std::map<int, int>>("block_to_materials",
                                              "A map from blocks to OpenMC materials.");
  return params;
}

CollisionKernel::CollisionKernel(const InputParameters & params)
  : IntegralRayKernelBase(params),
    _T(coupledValue("temperature")),
    _block_to_openmc_materials(getParam<std::map<int, int>>("block_to_materials"))
{
  //TODO Check that _T is a constant monomial
}

void
CollisionKernel::onSegment()
{
  // Create a fake neutron to compute the cross sections
  openmc::Particle p;
  p.sqrtkT_ = std::sqrt(openmc::K_BOLTZMANN * _T[0]);
  p.material_ = _block_to_openmc_materials.at(_current_elem->subdomain_id());
  p.coord_[p.n_coord_ - 1].cell = 0; // avoids a geometry search
  p.u() = {currentRay()->direction()(0), currentRay()->direction()(1), currentRay()->direction()(2)};
  p.E_ = currentRay()->auxData(0);

  p.event_calculate_xs();

  // Compute distance to next collision
  // p.event_advance();
  // scores tracklength tallies as well
  const Real collision_distance = -std::log(openmc::prn(p.current_seed())) /
      p.macro_xs_.total;

  // Shorter than next intersection, ray tracing will take care of moving the
  // particle
  if (collision_distance > _current_segment_length) {
    // p.event_cross_surface();
    //TODO Score track length tallies
    return;
  }
  else
  {
    // Advance ray (really, move backwards)
    Point current_position = currentRay()->currentPoint() -
        (_current_segment_length - collision_distance) * currentRay()->direction();

    // Compute collision
    p.event_collide();

    // Update Ray direction
    Point new_direction(p.u()[0], p.u()[1], p.u()[2]);
    std::cout << "Changing direction from " << currentRay()->direction() << " to " << new_direction << std::endl;
    std::cout << "Changing energy from " << currentRay()->auxData(0) << " to " << p.E_ << std::endl;
    std::cout << "new position " << current_position << std::endl;
    changeRayStartDirection(current_position, new_direction);

    // Update Ray energy
    currentRay()->auxData(0) = p.E_;
  }
}
