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
#include "openmc/nuclide.h"
#include "openmc/reaction.h"

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
  params.addRequiredParam<MooseEnum>(
      "granularity", granularity_types, "Scope of the tally value to retrieve");
  params.addRequiredParam<std::string>("score",
                                       "Score or reaction to retrieve wanted tally value from");
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
    _score(getParam<std::string>("score")),
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
  double val = 0;
  int filter_index;
  int score_index;
  int nuc_bin;

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

        // TODO: add the case where all nuclides need to be summed
        // TODO: add case where not all cells are in cell filters
        // can't index by element->id() anymore

        // check if individual nuclides are scored in tally and find the index
        if (t->nuclides_[0] != -1)
        {
          auto nuc_i = openmc::data::nuclide_map[_nuclide];
          auto it = find(t->nuclides_.begin(), t->nuclides_.end(), nuc_i);
          nuc_bin = it - t->nuclides_.begin();
        }
        else
          nuc_bin = 0;

        // find the score bin and stride length to get score index
        int mt = openmc::reaction_type(_score);
        auto it = find(t->scores_.begin(), t->scores_.end(), mt);
        int score_bin = it - t->scores_.begin();
        int score_stride = t->nuclides_.size();

        score_index = score_bin * score_stride + nuc_bin;

        // initialize indices for tally summing
        int univ_start = 0;
        int univ_end = 1;
        int univ_stride = 1;
        int energy_start = 0;
        int energy_end = 1;
        int energy_stride = 1;
        int cell_bin = _current_elem->id();
        int cell_stride = 1;

        // find the indices and strides needed to sum tally results
        for (auto i = 0; i < t->filters().size(); ++i)
        {
          auto i_filt = t->filters(i);
          if (openmc::model::tally_filters[i_filt]->type() == "universe")
          {
            univ_end = openmc::model::tally_filters[i_filt]->n_bins();
            univ_stride = t->strides(i);
          }
          else if (openmc::model::tally_filters[i_filt]->type() == "energy")
          {
            if (_all_energies)
            {
              energy_end = openmc::model::tally_filters[i_filt]->n_bins();
            }
            else
            {
              energy_start = _energy_bin;
              energy_end = energy_start + 1;
            }
            energy_stride = t->strides(i);
          }
          else if (openmc::model::tally_filters[i_filt]->type() == "cell")
          {
            cell_stride = t->strides(i);
          }
        }

        for (int i = univ_start; i < univ_end; ++i)
        {
          for (int j = energy_start; j < energy_end; ++j)
          {
            filter_index = i * univ_stride + cell_bin * cell_stride + j * energy_stride;
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
