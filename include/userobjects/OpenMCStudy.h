//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "RayTracingStudy.h"

// openmc includes
#include "openmc/capi.h"

/**
 * Class to use the Monte Carlo method for neutron transport
 *
 */
class OpenMCStudy : public RayTracingStudy
{
public:
  OpenMCStudy(const InputParameters & parameters);

  static InputParameters validParams();

protected:

  void generateRays();

private:

};
