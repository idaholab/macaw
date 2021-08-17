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

  void finalize(){};

protected:
  // TODO Add docstrings

  // Id to reference in TallyAux
  int _id;

  // Particle type to tally values for
  const MooseEnum _particle;

  // Estimator type for tally
  const MooseEnum _estimator;

  // Scores and reactions to tally
  std::vector<std::string> _scores;

  // Filters to use for the tally
  std::vector<std::string> _filters;

  // Nuclides to tally reactions on
  std::vector<std::string> _nuclides;

  // Energy bin edges for energy filter
  std::vector<Real> _energy_bins;

  // Cells to use in cell filter
  std::vector<int> _cell_bins;

  // Blocks to use in universe filter
  std::vector<int> _block_bins;
};

/**
 * Comparison funtion for a custom sort of the specified Filters
 *so that tally values can be systmatically retieved easily in OpenMCTallyAux
 */
bool
cmp(std::string x, std::string y)
{
  if (x == "universe")
    return true;
  if (y == "universe")
    return false;
  if (x == "cell")
    return true;
  if (y == "cell")
    return false;
  if (x == "energy")
    return true;
  if (y == "energy")
    return false;
  else
    return false;
}
