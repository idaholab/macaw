//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Local includes
#include "OpenMCStudy.h"

registerMooseObject("RayTracingApp", OpenMCStudy);

InputParameters
OpenMCStudy::validParams()
{
  auto params = RayTracingStudy::validParams();
  params.addClassDescription("Runs OpenMC like a bawsse.");
  return params;
}

OpenMCStudy::OpenMCStudy(const InputParameters & params)
  : RayTracingStudy(params)
{

// A place to think out loud:
/*
This class doesnt really need to get anything that can be in OpenMC input files:
- materials
- settings (number of neutrons, batches, etc)
- even tallies (for now, we'll just replicated)!

Its real objective should be to call each part of openmc_run, and replace just the core stuff with 'Ray' instead of neutrons.
Maybe we could even make a subclass of 'Particle, neutron', that also inherits from 'Ray' so that both OpenMC and MOOSE know what to do with it

There are a few transfers we want to make:
- temperature, MAYBE. It could be handled on moose side! (and sent to OpenMC via Particle T attribute)
- power distribution tally, from an OpenMC tally to MOOSE

LONG TERM:
- moose need to handle the tallies, because we need domain decomposition
- more than 1 batch ? If it's annoying
*/

// Idea : run OpenMC initialization in constructor


}


void OpenMCStudy::generateRays()
{

}

// Execute: rewrite openmc_run but plug in the Rays
