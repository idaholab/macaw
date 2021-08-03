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

  params.addClassDescription("Retrieves tally information from OpenMC");

  MooseEnum granularity_types("global universe cell");
  MooseEnum estimator_types("ANALOG TRACKLENGTH COLLISION", "COLLISION");
  MooseEnum particle_types("neutron photon electron positron", "neutron");

  params.addParam<int>("tally_id", "Tally id used to access tally");
  params.addRequiredParam<MooseEnum>("granularity", granularity_types, "Scope of the tally value to retrieve");
  params.addRequiredParam<MooseEnum>("estimator", estimator_types, "Estimator type of the tally");
  params.addRequiredParam<MooseEnum>("particle_type", particle_types, "Particle type of the tally");
  params.addParam<std::string>("nuclide", "Nuclide to get tally value for");
  params.addParam<int>("energy_bin", "Energy_bin to get tally value for");
  // params.addRequiredParam<int>("filter", "Filter id used to get tally value");

  // TODO: Add all filters and scores to retrieve in the right bin
  // TODO: Add option to compute std deviation

  return params;
}

OpenMCTallyAux::OpenMCTallyAux(const InputParameters & params)
  : AuxKernel(params),
    _retrieve_from_tally_id(isParamValid("tally_id")),
    _tally_id(isParamValid("tally_id") ? getParam<int>("tally_id") : -1),
    _granularity(getParam<MooseEnum>("granularity")),
    _particle(getParam<MooseEnum>("particle_type")),
    _estimator(getParam<MooseEnum>("estimator")),
    _all_nuclides(!isParamValid("nuclide")),
    _nuclide(isParamValid("nuclide") ? getParam<std::string>("nuclide") : "all"),
    _all_energies(!isParamValid("energy_bin")),
    _energy_bin(isParamValid("energy_bin") ? getParam<int>("energy_bin") : -1)

//_filter(getParam<int>("filter"))
{
  // if (_estimator != openmc::model::tallies[openmc::model::tally_map[_tally_id]]->estimator_)
  //   paramError("estimator", "estimator does not match the estimator of given tally");
}

double
OpenMCTallyAux::computeValue()
{
  double val;
  int filter_index;
  int score_index;

  if (_retrieve_from_tally_id)
  {
    // Retrieve tally based on the tally id
    auto & t = openmc::model::tallies[openmc::model::tally_map[_tally_id]];

    switch (_granularity)
    {
      case 0:
      // insert global
        mooseError("Global Tally retrieving currently not supported");
        break;
      case 1:
        // insert universe
        mooseError("Universe Tally retrieving currently not supported");
        break;
      case 2:

        if (t->nuclides_[0] == -1) int score_index = 0;
        else
          mooseError("Individual nuclide tallying is not available yet");
        int univ_bins = 1;
        int univ_stride = 1;
        int energy_bins = 1;
        int energy_stride =1;
        int cell_bin = _current_elem->id();
        int cell_stride = 1;

        for (auto i = 0; i < t->filters().size(); ++i)
        {
          auto i_filt = t->filters(i);
          if (openmc::model::tally_filters[i_filt]->type() == "universe"){
            univ_bins = openmc::model::tally_filters[i_filt]->n_bins();
            univ_stride = t->strides(i);
          }
          else if (openmc::model::tally_filters[i_filt]->type() == "energy")
          {
            energy_bins = openmc::model::tally_filters[i_filt]->n_bins();
            energy_stride = t->strides(i);
          }
          else if (openmc::model::tally_filters[i_filt]->type() == "cell"){
            cell_stride = t->strides(i);
          }
        }


        for (int i = 0; i < univ_bins; ++i)
        {
          for (int j = 0; j < energy_bins; ++j)
          {
            filter_index = i*univ_stride + cell_bin*cell_stride + j*energy_stride;
            val += xt::view(t->results_, filter_index, score_index, 1);
          }
        }

        break;
    }


    return val;
  }
  else
  {
    mooseError("Use tally ids, for now");
    return 0;
  }
}
