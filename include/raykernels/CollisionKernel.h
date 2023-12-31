//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegralRayKernelBase.h"
#include "MooseVariableInterface.h"

// openmc includes
#include "openmc/particle.h"

class CollisionKernel : public IntegralRayKernelBase
{
public:
  CollisionKernel(const InputParameters & params);

  static InputParameters validParams();

  void initialSetup();

  void onSegment();

protected:
  /// Holds the temperature in the current element
  const VariableValue & _T;

  /// Map from blocks to OpenMC materials
  std::unordered_map<int, int> _block_to_openmc_materials;

  /// OpenMC particle objects to call openmc routines with
  std::vector<openmc::Particle> _particles;

  /// Whether the mesh is 2D
  const bool _is_2D;

  /// Z-coordinate for 2D calculations
  const Real _z_coord;

  /// whether to print detailed collision information
  const bool _verbose;
};
