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

class CollisionKernel : public IntegralRayKernelBase
{
public:
  CollisionKernel(const InputParameters & params);

  static InputParameters validParams();

  void onSegment();

protected:
  /// Holds the temperature in the current element
  const VariableValue & _T;

  // Map from blocks to OpenMC materials
  const std::map<int, int> _block_to_openmc_materials;
};
