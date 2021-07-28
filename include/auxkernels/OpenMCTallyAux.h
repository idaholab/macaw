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

// TODO Add docstring
class OpenMCTallyAux : public AuxKernel
{
public:
  static InputParameters validParams();

  OpenMCTallyAux(const InputParameters & params);

protected:
  virtual Real computeValue() override;

  // TODO Add docstring
  // Whether to use the tally id to retrieve the tally we want to plot
  const bool _retrieve_from_tally_id;
  //  int _filter

  int _tally_id;
};
