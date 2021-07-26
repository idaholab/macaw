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

  params.addParam<int>("tally_id", "Tally id used to access tally");
  // params.addRequiredParam<int>("filter", "Filter id used to get tally value");

  // TODO: Add all filters and scores to retrieve in the right bin
  // TODO: Add option to compute std deviation

  return params;
}

OpenMCTallyAux::OpenMCTallyAux(const InputParameters & params)
  : AuxKernel(params),
  _retrieve_from_tally_id(isParamValid("tally_id")),
  _tally_id(isParamValid("tally_id") ? getParam<int>("tally_id") : -1)
//_filter(getParam<int>("filter"))
{
}

double
OpenMCTallyAux::computeValue()
{
  if (_retrieve_from_tally_id)
  {
    // Retrieve tally based on the tally id
    auto & t = openmc::model::tallies[_tally_id - 1];
    //FIXME This is not correct. You need to retrieve the index into this array, which is not the id

    // Compute filter index from specified filters and tally filters
    const int filter_index = _current_elem->id();

    // Compute score index from specified score
    const int score_index = 0;

    // Retrieve value
    auto val = xt::view(t->results_, filter_index, score_index, 1);
    _console << _current_elem->id() << " " << val << std::endl;


    // _console << "tally vector length: " << openmc::model::tallies.size() << std::endl;
    // auto shape = t->results_.shape();
    // _console << xt::adapt(shape) << std::endl;

    return val;
  }
  else
  {
    mooseError("Use tally ids, for now");
    return 0;
  }
}
