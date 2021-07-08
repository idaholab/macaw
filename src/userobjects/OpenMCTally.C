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
#include "openmc/particle_data.h"

#include <string>

registerMooseObject("MaCawApp", OpenMCTally);

InputParameters
OpenMCTally::validParams()
{

  InputParameters params = GeneralUserObject::validParams();

  // scores/filters/nuclides/estimators/type/particle/reaction_type


  params.addParam<MooseEnum>("particle_type",getParticleTypeEnum(),"particle type to track in tally");
  params.addParam<MooseEnum>("tally_estimator", getTallyEstimatorEnum(), "Estimator type used for the tally");
  params.addParam<std::vector<std::string>>("tally_scores","Scores to apply to the tally");
  //params.addParam<std::vector<openmc::ReactionType>>("tally_reaction_types","MT reactions to tally");
  params.addParam<std::vector<std::string>>("tally_filters","Filters to apply to the tally");
  params.addParam<std::vector<Real>>("tally_energy_bins","Define energy bins for the tally");

  return params;
}

OpenMCTally::OpenMCTally(const InputParameters & params)
  : GeneralUserObject(params),
  _tally_particle(getParam<MooseEnum>("particle_type").getEnum<particles::ParticleEnum>()),
  _tally_estimator(getParam<MooseEnum>("tally_estimator").getEnum<estimators::TallyEstimatorEnum>()),
  _tally_scores(getParam<std::vector<std::string>>("tally_scores")),
//  _tally_reaction_types(getParam<std::vector<std::string>>("tally_reaction_types")),
  _tally_filters(getParam<std::vector<std::string>>("tally_filters")),
  _tally_energy_bins(getParam<std::vector<Real>>("tally_energy_bins"))
{

}

void
OpenMCTally::execute()
{
  using namespace openmc;

  if (_tally_particle != particles::neutron) {
    paramError("particle_type", "Only neutrons are currently supported.");
  }

  // create a new tally with auto id
  std::cout << "Creating new tally" << std::endl;
  model::tallies.push_back(make_unique<Tally>(C_NONE));

  // TODO: check for a mesh parameter and add mesh if exists


  //create vector of filters to apply to tally
  vector<Filter*> filters;

  std::cout << "Adding tally filters" << std::endl;
  for(int i = 0; i < _tally_filters.size(); ++i){
    //create filter and add to filters vector
    // create takes in string argument
    // make filter param MooseEnum?
    // make method to get string given enum to git rid of if/switch statments?

    Filter* filter_ptr = Filter::create(_tally_filters.at(i), C_NONE);

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
    }

    filters.push_back(filter_ptr);

  }
  // appply filters to new tally
  model::tallies.back()->set_filters(filters);

  // apply scores
  std::cout << "Adding tally scores" << std::endl;
  model::tallies.back()->set_scores(_tally_scores);

  // set the tally estimator
  // get rid of switch with enum to string method?
  std::cout << "Adding tally estimator" << std::endl;
  switch (_tally_estimator)
  {
    case estimators::ANALOG:
    {
      model::tallies.back()->estimator_ = TallyEstimator::ANALOG;
    }

    case estimators::TRACKLENGTH:
    {
      model::tallies.back()->estimator_ = TallyEstimator::TRACKLENGTH;
    }

    case estimators::COLLISION:
    {
      model::tallies.back()->estimator_ = TallyEstimator::COLLISION;
    }
  }
  std::cout << "id: " << model::tallies.back()->id_ << std::endl;
}

void OpenMCTally::initialize() {};

void OpenMCTally::finalize() {};

void OpenMCTally::threadJoin() {};
