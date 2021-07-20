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
#include "openmc/particle_data.h"

#include <string>

registerMooseObject("MaCawApp", OpenMCTally);

InputParameters
OpenMCTally::validParams()
{

  InputParameters params = GeneralUserObject::validParams();

  // scores/filters/nuclides/estimators/type/particle/reaction_type
  MooseEnum estimator_types("ANALOG TRACKLENGTH COLLISION", "COLLISION");
  MooseEnum particle_types("neutron photon electron positron", "neutron");

  params.addRequiredParam<int>("tally_id", "Tally id used to extract tally from an auxkernal")
  params.addParam<MooseEnum>("particle_type", particle_types,"particle type to track in tally");
  params.addParam<MooseEnum>("tally_estimator", estimator_types, "Estimator type used for the tally");
  params.addRequiredParam<std::vector<std::string>>("tally_scores","Scores to apply to the tally");
  params.addRequiredParam<std::vector<std::string>>("tally_filters","Filters to apply to the tally");
  params.addRequiredParam<std::vector<int>>("filter_ids","Filter ids used to extract tally from an auxkernal")
  params.addParam<std::vector<Real>>("tally_energy_bins","Define energy bins for the tally");

  return params;
}

OpenMCTally::OpenMCTally(const InputParameters & params)
  : GeneralUserObject(params),
  _tally_id(getParam<int>("tally_id"))''
  _tally_particle(getParam<MooseEnum>("particle_type")),
  _tally_estimator(params.get<MooseEnum>("tally_estimator")),
  _tally_scores(getParam<std::vector<std::string>>("tally_scores")),
  _tally_filters(getParam<std::vector<std::string>>("tally_filters")),
  _filter_ids(getParam<std::vector<int>>("filter_ids")),
  _tally_energy_bins(getParam<std::vector<Real>>("tally_energy_bins"))
{
  if (_tally_particle != 0) {
    paramError("particle_type", "Only neutrons are currently supported.");
  }

  if (_tally_estimator != 2){
    paramError("tally_estimator", "Only collision estimator currently supported");
  }
  if (!_execute_enum.contains(EXEC_INITIAL) || _execute_enum.size() > 1){
    paramError("execute_on", "execute_on must be INITIAL to ensure tallies are created "
     "once at the beginning of the simulation");
  }
}

void
OpenMCTally::initialize()
{
  using namespace openmc;

  // create a new tally with auto id
  std::cout << "Creating new tally" << std::endl;
  model::tallies.push_back(make_unique<Tally>(_tally_id));

  // TODO: check for a mesh parameter and add mesh if exists


  //create vector of filters to apply to tally
  vector<Filter*> filters;


  for(int i = 0; i < _tally_filters.size(); ++i){
    //create filter and add to filters vector
    // create takes in string argument
    // make filter param MooseEnum?
    // make method to get string given enum to git rid of if/switch statments?

    Filter* filter_ptr = Filter::create(_tally_filters.at(i), _filter_ids.at(i));

    if (filter_ptr->type() == "energy" ){
      std::cout << " Adding energy filter" << std::endl;

      EnergyFilter* energy_filter = dynamic_cast<EnergyFilter*>(filter_ptr);
      energy_filter->set_bins(_tally_energy_bins);

    } else if (filter_ptr->type() == "particle"){
      std::cout << " Adding particle filter" << std::endl;

      ParticleFilter* particle_filter = dynamic_cast<ParticleFilter*>(filter_ptr);
      // must be put in a vector to pass into set_particles
      vector<ParticleType> types;
      types.push_back(ParticleType::neutron);
      particle_filter->set_particles(types);

    } else if (filter_ptr->type() == "universe"){
      std::cout << " Adding universe filter" << std::endl;

      UniverseFilter* universe_filter = dynamic_cast<UniverseFilter*>(filter_ptr);

      vector<int> universe_ids;
      for(int i = 0; i < model::universes.size(); ++i)
        universe_ids.push_back(i);

      universe_filter->set_universes(universe_ids);

    }

    else
      mooseError("Unrecognized filter");

    filters.push_back(filter_ptr);

  }
  // appply filters to the new tally
  model::tallies.back()->set_filters(filters);

  // apply scores
  std::cout << "Adding tally scores" << std::endl;
  model::tallies.back()->set_scores(_tally_scores);

  // set the tally estimator
  // get rid of switch with enum to string method?
  std::cout << "Adding tally estimator" << std::endl;
  switch (_tally_estimator)
  {
    case 0:
    {
      model::tallies.back()->estimator_ = TallyEstimator::ANALOG;
      break;
    }

    case 1:
    {
      model::tallies.back()->estimator_ = TallyEstimator::TRACKLENGTH;
      break;
    }

    case 2:
    {
      model::tallies.back()->estimator_ = TallyEstimator::COLLISION;
      break;
    }
    default:
      mooseError("Unrecognized estimator");
      // TODO This is being triggered so the input of the estimator must have an issue
  }

  for (auto& t : model::tallies) {
    t->init_results();
  }

}

void OpenMCTally::execute() {};

void OpenMCTally::finalize() {};

void OpenMCTally::threadJoin() {};
