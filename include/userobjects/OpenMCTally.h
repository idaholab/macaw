//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

#include "openmc/constants.h"

// Forward Declarations
class InputParameters;

/**
 * User object intermediate base class that declares an interface for providing generic fields
 * by name. Note: This object is intentionally inherited from GeneralUserObject (not elemental)
 * because several cases exist where we need to perform more complex operations possibly over
 * non-local elements
 */
class OpenMCTally : public GeneralUserObject
{
public:
  static InputParameters validParams();

  OpenMCTally(const InputParameters & parameters);

  void execute();

  void initialize();

  void finalize();

  void threadJoin();

protected:
  int _tally_id;

  const MooseEnum _tally_particle;

  const MooseEnum _tally_estimator;

  std::vector<std::string> _tally_scores;

  std::vector<std::string> _tally_filters;

  std::vector<int> _filter_ids;

  std::vector<Real> _tally_energy_bins;

  //  virtual unsigned long getElementalValueLong(dof_id_type /*element_id*/,
  //                                              const std::string & /*field_name*/) const
  //  {
  //    mooseError(name(), " does not satisfy the getElementalValueLong interface");
  //  }

  //  virtual Real getElementalValueReal(dof_id_type /*element_id*/,
  //                                     const std::string & /*field_name*/) const
  //  {
  //    mooseError(name(), " does not satisfy the getElementalValueReal interface");
  //  }
};
