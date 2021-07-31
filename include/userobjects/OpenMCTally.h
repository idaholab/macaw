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
 * Adds a tally to the OpenMC simulation.
 */
class OpenMCTally : public GeneralUserObject
{
public:
  static InputParameters validParams();

  OpenMCTally(const InputParameters & parameters);

  // Set up filters, bins and initialize tallies
  void initialize();

  void execute(){};

  void threadJoin(){};

  void finalize(){};

protected:
  // TODO Add docstrings
  int _id;

  const MooseEnum _particle;

  const MooseEnum _estimator;

  std::vector<std::string> _scores;

  std::vector<std::string> _filters;

  std::vector<int> _filter_ids;

  std::vector<Real> _energy_bins;

  std::vector<int> _cell_bins;

  std::vector<int> _block_bins;
};
