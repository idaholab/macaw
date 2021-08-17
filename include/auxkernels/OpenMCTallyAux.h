//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Retrieves tally values from OpenMC tallies and tranfers the values
 * to a MOOSE auxiliary variable
 */
class OpenMCTallyAux : public AuxKernel
{
public:
  static InputParameters validParams();

  OpenMCTallyAux(const InputParameters & params);

protected:
  virtual Real computeValue() override;

  /// Whether to use the tally id to retrieve the tally we want to plot
  const bool _retrieve_from_tally_id;

  /// Id of the tally from which the desired score is retrieved
  int _tally_id;

  /// Spatial scope of the tally values to retrieve
  const MooseEnum _granularity;

  /// Score or reaction to retrieve from tally
  std::string _score;

  /// Particle used in tally to filter events
  const MooseEnum _particle;

  /// Estimator used to tally
  const MooseEnum _estimator;

  /// Whether to sum tally values over all nuclides
  const bool _all_nuclides;

  /// Nuclide to retrieve tally value from
  std::string _nuclide;

  /// Whether to sum tally values over all energies
  const bool _all_energies;

  /// Energy bin to retrieve tally value from
  int _energy_bin;
};
