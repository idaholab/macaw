//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OpenMCTally.h"
#include "openmc/constants.h"
#include "openmc/tallies/tally.h"
#include "openmc/tallies/filter.h"
#include "openmc/tallies/filter_energy.h"
#include "openmc/tallies/filter_particle.h"
#include "openmc/tallies/filter_universe.h"
#include "openmc/tallies/filter_cell.h"
#include "openmc/particle_data.h"

#include <xtensor/xio.hpp>
#include <xtensor/xtensor.hpp>
#include <xtensor/xarray.hpp>

#include <string>

registerMooseObject("MaCawApp", OpenMCTally);

InputParameters
OpenMCTally::validParams()
{

  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Adds a tally to the simulation.");

  // scores/filters/nuclides/estimators/type/particle/reaction_type
  MooseEnum estimator_types("ANALOG TRACKLENGTH COLLISION", "COLLISION");
  MooseEnum particle_types("neutron photon electron positron", "neutron");

  params.addParam<MooseEnum>(
      "estimator", estimator_types, "Estimator type, defaults to a collision estimator");
  params.addRequiredParam<std::vector<std::string>>("scores", "Scores of the tally");

  // Filters and bins
  params.addParam<MooseEnum>("particle_type", particle_types, "Particle type to track in tally");
  params.addRequiredParam<std::vector<std::string>>("filters", "Filters to apply to the tally");
  params.addParam<std::vector<Real>>("energy_bins", "Energy bins");
  params.addParam<std::vector<Real>>("cell_bins",
      "Mesh cell bins. Defaults to all cells if a CellFilter is present");
  params.addParam<std::vector<Real>>("block_bins",
      "Mesh block bins. Defaults to all blocks if a UniverseFilter is present");

  // TODO Make particle filter accept more than one type (for example 2 bins: neutron and photon)
  // TODO Add nuclide filter & bins
  // TODO Add reaction filter & bins (or are they scores?)
  // TODO Add structured mesh filter
  // TODO Add Functional Expansion filters

  // TODO : Try not to use that, user should not input the id of a tally. What if it s the same
  // as an exisiting one?
  params.addRequiredParam<int>("id", "Tally id used to extract tally from an auxkernel");


  return params;
}

OpenMCTally::OpenMCTally(const InputParameters & params)
  : GeneralUserObject(params),
    _id(getParam<int>("id")),
    _particle(getParam<MooseEnum>("particle_type")),
    _estimator(getParam<MooseEnum>("estimator")),
    _scores(getParam<std::vector<std::string>>("scores")),
    _filters(getParam<std::vector<std::string>>("filters")),
    _energy_bins(isParamValid("energy_bins") ?
        getParam<std::vector<Real>>("energy_bins") : std::vector<Real>()),
    _cell_bins(isParamValid("cell_bins") ?
        getParam<std::vector<int>>("cell_bins") : std::vector<int>()),
    _block_bins(isParamValid("block_bins") ?
        getParam<std::vector<int>>("block_bins") : std::vector<int>())
{
  // Parameter checks
  if (_particle != 0)
    paramError("particle_type", "Only neutrons are currently supported.");
  if (_estimator != 2)
    paramError("estimator", "Only collision estimator currently supported");
  if (!_execute_enum.contains(EXEC_INITIAL) || _execute_enum.size() > 1)
    paramError("execute_on", "execute_on must be INITIAL to ensure tallies are created "
               "once at the beginning of the simulation");

  // Check that bins are specified for the energy filter if required
  if (std::find(_filters.begin(), _filters.end(), "energy") != _filters.end() &&
    !isParamValid("energy_bins"))
    mooseError("The energy bins parameter should be supplied if an energy filter is "
               "requested.");
}

void
OpenMCTally::initialize()
{
  using namespace openmc;

  // Create a new tally (with specified id -> not a fan of this)
  _console << "Creating new tally" << std::endl;
  model::tallies.push_back(make_unique<Tally>(_id));

  // TODO: check for a mesh parameter and add mesh if exists


  // Create vector of tally and add each one from the _filters parameters
  vector<Filter*> filters;
  for (unsigned int i = 0; i < _filters.size(); ++i)
  {
    Filter * filter_ptr = Filter::create(_filters.at(i), C_NONE);

    // Add the filters and set the bins from the parameters
    if (filter_ptr->type() == "energy")
    {
      _console << " Adding energy filter" << std::endl;

      EnergyFilter* energy_filter = dynamic_cast<EnergyFilter*>(filter_ptr);
      energy_filter->set_bins(_energy_bins);

    }
    else if (filter_ptr->type() == "particle")
    {
      _console << " Adding particle filter" << std::endl;

      ParticleFilter* particle_filter = dynamic_cast<ParticleFilter*>(filter_ptr);

      vector<ParticleType> types(_particle);
      particle_filter->set_particles(types);
    }
    else if (filter_ptr->type() == "universe")
    {
      _console << " Adding universe filter" << std::endl;

      UniverseFilter * universe_filter = dynamic_cast<UniverseFilter *>(filter_ptr);

      // Default is every block
      vector<int> universe_ids;
      if (!isParamValid("block_bins"))
        for (int i = 0; i < model::universes.size(); ++i)
          universe_ids.push_back(i);
      else
        universe_ids = _block_bins;

      universe_filter->set_universes(universe_ids);
    }
    else if (filter_ptr->type() == "cell")
    {
      _console << " Adding cell filter" << std::endl;

      CellFilter * cell_filter = dynamic_cast<CellFilter *>(filter_ptr);

      // Default is every cell
      vector<int> cell_ids;
      if (!isParamValid("cell_bins"))
        for (int i = 0; i < model::cells.size(); ++i)
          cell_ids.push_back(i);
      else
        cell_ids = _cell_bins;

      cell_filter->set_cells(cell_ids);
    }
    else
      mooseError("Unrecognized filter");

    filters.push_back(filter_ptr);
  }
  // Add filters to the new tally
  model::tallies.back()->set_filters(filters);

  // Add scores
  model::tallies.back()->set_scores(_scores);

  // Add nuclides //TODO
  vector<std::string> nuc;
  nuc.push_back("total");
  model::tallies.back()->set_nuclides(nuc);

  // Set the tally estimator
  // get rid of switch with enum to string method?
  switch (_estimator)
  {
    case 0:
      model::tallies.back()->estimator_ = TallyEstimator::ANALOG;
      break;
    case 1:
      model::tallies.back()->estimator_ = TallyEstimator::TRACKLENGTH;
      break;
    case 2:
      model::tallies.back()->estimator_ = TallyEstimator::COLLISION;
      break;
    default:
      mooseError("Unrecognized estimator");
  }

  // TODO: Only do this for the new tallies
  // Allocate then initialize tally results arrays

  model::tallies.back()->init_results();
  model::tallies.back()->results_.fill(0);
  // model::tallies.back()->results_(0,0,0) = 1.1;
  std::cout << model::tallies.back()->results_ << std::endl;
  std::cout << model::tallies.size() << std::endl;

  // Add new user tallies to active tallies list
  openmc::setup_active_tallies();
}
