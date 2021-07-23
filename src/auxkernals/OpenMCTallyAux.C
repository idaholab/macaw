//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OpenMCTallyAux.h"

#include "openmc/tallies/tally.h"

#include <xtensor/xarray.hpp>
#include <xtensor/xfixed.hpp>
#include <xtensor/xio.hpp>
#include <xtensor/xtensor.hpp>
#include <xtensor/xview.hpp>

registerMooseObject("MaCawApp", OpenMCTallyAux);

InputParameters
OpenMCTallyAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addRequiredParam<int>("tally_id", "Tally id used to access tally");
  //params.addRequiredParam<int>("filter", "Filter id used to get tally value");

  return params;
}

OpenMCTallyAux::OpenMCTallyAux(const InputParameters & params)
  : AuxKernel(params),
  _tally_id(getParam<int>("tally_id"))
  //_filter(getParam<int>("filter"))
{
}

double
OpenMCTallyAux::computeValue()
{
  if (isNodal())
    mooseError("This AuxKernel only supports Elemental fields");
  auto& t = openmc::model::tallies[_tally_id -1 ];
  auto val = xt::view(t->results_,_current_elem->id(),0,1);
  //auto val = xt::sum(elem_scores)();
  std::cout << _current_elem->id() << " " << val << std::endl;
  // std::cout << "tally vector length: " << openmc::model::tallies.size() << std::endl;
  // auto shape = t->results_.shape();
  // std::cout << xt::adapt(shape) << std::endl;

  return val; //5.0;
}
