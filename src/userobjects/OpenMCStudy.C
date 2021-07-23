//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Local includes
#include "OpenMCStudy.h"

// MOOSE includes
#include "TimedPrint.h"
#include "Transient.h"

// openmc includes
#ifdef OPENMC_MPI
#include <mpi.h>
#endif
#include "openmc/capi.h"
#include "openmc/constants.h"
#include "openmc/error.h"
#include "openmc/message_passing.h"
#include "openmc/particle_restart.h"
#include "openmc/settings.h"
#include "openmc/particle.h"
#include "openmc/cell.h"
#include "openmc/geometry.h"

// additional includes if want to decompose initialization
#include "openmc/simulation.h"
#include "openmc/message_passing.h"
#include "openmc/state_point.h"
#include "openmc/material.h"
#include "openmc/output.h"
#include "openmc/finalize.h"
#include "openmc/source.h"
#include "openmc/bank.h"

registerMooseObject("MaCawApp", OpenMCStudy);

InputParameters
OpenMCStudy::validParams()
{
  auto params = RayTracingStudy::validParams();
  params.addClassDescription("Runs OpenMC like a bawsse.");

  // Neutrons dont really need names?
  params.addPrivateParam<bool>("_use_ray_registration", false);

  return params;
}

OpenMCStudy::OpenMCStudy(const InputParameters & params)
  : RayTracingStudy(params),
  _rays(declareRestartableDataWithContext<std::vector<std::shared_ptr<Ray>>>("rays", this)),
  _local_rays(
      declareRestartableDataWithContext<std::vector<std::shared_ptr<Ray>>>("local_rays", this)),
  _claim_rays(*this, _rays, _local_rays, true),
  _claim_rays_timer(registerTimedSection("claimRays", 1)),
  _define_rays_timer(registerTimedSection("defineRays", 1))
{

/*
This class doesnt really need to get anything that can be in OpenMC input files:
- materials
- settings (number of neutrons, batches, etc)
- even tallies (for now, we'll just replicated)!

Its real objective should be to call each part of openmc_run, and replace just the core stuff with 'Ray' instead of neutrons.
Maybe we could even make a subclass of 'Particle, neutron', that also inherits from 'Ray' so that both OpenMC and MOOSE know what to do with it

There are a few transfers we want to make:
- temperature, MAYBE. It could be handled on moose side! (and sent to OpenMC via Particle T attribute)
- power distribution tally, from an OpenMC tally to MOOSE

LONG TERM:
- moose need to handle the tallies, because we need domain decomposition
- more than 1 batch ? If it's annoying

How to handle OpenMC physics calls
// save the OpenMC neutron in AuxData
// OR
// make a class that inherits from both
// OR
// use fake neutrons to call the routines and move results over
// OR
// use fake neutrons created once and use references to move results over
*/

  // Initialize run
  char * argv[1] = {(char*)"openmc"};
  int err = openmc_init(1, argv, &_communicator.get());
  if (err)
    openmc::fatal_error(openmc_err_msg);

  // Check run mode
  if (openmc::settings::run_mode == openmc::RunMode::PARTICLE ||
      openmc::settings::run_mode == openmc::RunMode::PLOTTING ||
      openmc::settings::run_mode == openmc::RunMode::VOLUME)
    mooseError("Requested run mode is currently not supported by MaCaw");

  openmc_simulation_init();

  openmc::model::n_coord_levels = 1;
  // resize the number of cells in openmc to the number of elements in moose
  std::cout << "Number of Mesh Elements: " << _mesh.nElem() << std::endl;
  openmc::model::cells.resize(_mesh.nElem());
  std::cout << "Resizing number of OpenMC cells to: " << openmc::model::cells.size() << std::endl;

  openmc::model::cell_map.clear();

  for (int i = 0; i < openmc::model::cells.size(); ++i)
  {
    openmc::model::cells[i] = gsl::make_unique<openmc::CSGCell>();
    openmc::model::cells[i]->id_ = i;
    openmc::model::cells[i]->universe_ = _mesh.elemPtr(i)->subdomain_id();
    openmc::model::cell_map[i] = i;
  }

  for (int i = 0; i < openmc::model::cells.size(); i++) {
    int32_t uid = openmc::model::cells[i]->universe_;
    auto it = openmc::model::universe_map.find(uid);
    if (it == openmc::model::universe_map.end()) {
      openmc::model::universes.push_back(gsl::make_unique<openmc::Universe>());
      openmc::model::universes.back()->id_ = uid;
      openmc::model::universes.back()->cells_.push_back(i);
      openmc::model::universe_map[uid] = openmc::model::universes.size() - 1;
    } else {
      openmc::model::universes[it->second]->cells_.push_back(i);
    }
  }

/*
  // resize the number of universes in openmc to the number of subdomains in moose
  std::cout << "Number of Mesh Subdomains: " << _mesh.meshSubdomains().size() << std::endl;
  openmc::model::universes.resize(_mesh.meshSubdomains().size());
  std::cout << "Resizing number of OpenMC universes to: " << openmc::model::universes.size() << std::endl;



  for (int i = 0; i < openmc::model::universes.size(); ++i){
    openmc::model::universes[i] = gsl::make_unique<openmc::Universe>();
    openmc::model::universes[i]->id_ = i;
  }
  */
  // does data OK
  // work TODO -> no need here
  // banks TODO -> no need for us
  // tally initialization TODO
  // nuclide index mapping OK

  // TODO Initialize this nicer
  registerRayAuxData("energy");
  registerRayAuxData("weight");

  // Set the number of steps of the Transient executioner as the number of batches
  if (dynamic_cast<Transient *>(_app.getExecutioner()))
    dynamic_cast<Transient *>(_app.getExecutioner())->forceNumSteps(openmc::settings::n_batches);
  else
    mooseWarning("Unable to set the number of batches for a non Transient Executioner");

  /* LONG TERM : REDO INITIALIZATION AND SKIP WHAT ISNT NECESSARY */
  // // Initialize nuclear data (energy limits, log grid)
  // if (openmc::settings::run_CE) {
  //   initialize_data();
  // }
  //
  // // Determine how much work each process should do
  // openmc::calculate_work();  // for bank sizes, just REDO
  //
  // // Allocate source, fission and surface source banks.
  // openmc::allocate_banks();  // REDO, all dynamic! =_rays?
  //
  // // Allocate tally results arrays if they're not allocated yet
  // // for (auto& t : openmc::model::tallies) {
  // //   t->init_results();  // REDO
  // // }
  //
  // // Set up material nuclide index mapping
  // for (auto& mat : openmc::model::materials) {
  //   mat->init_nuclide_index();
  // }
  //
  // // Reset global variables -- this is done before loading state point (as that
  // // will potentially populate k_generation and entropy)
  // openmc::simulation::current_batch = 0;
  // openmc::simulation::k_generation.clear();
  // // openmc::simulation::entropy.clear();
  // openmc_reset();
  //
  // // If this is a restart run, load the state point data and binary source
  // // file
  // if (openmc::settings::restart_run) {
  //   openmc::load_state_point();
  //   openmc::write_message("Resuming simulation...", 6);
  // } else {
  //   // Only initialize primary source bank for eigenvalue simulations
  //   if (openmc::settings::run_mode == openmc::RunMode::EIGENVALUE) {
  //     openmc::initialize_source();
  //   }
  // }
  //
  // // Display header
  // if (openmc::mpi::master) {
  //   if (openmc::settings::run_mode == openmc::RunMode::FIXED_SOURCE) {
  //     openmc::header("FIXED SOURCE TRANSPORT SIMULATION", 3);
  //   } else if (openmc::settings::run_mode == openmc::RunMode::EIGENVALUE) {
  //     openmc::header("K EIGENVALUE SIMULATION", 3);
  //     if (openmc::settings::verbosity >= 7) openmc::print_columns();
  //   }
  // }
  //
  // // Set flag indicating initialization is done
  // openmc::simulation::initialized = true;
}


OpenMCStudy::~OpenMCStudy()
{
  // Finalize and free up memory
  std::cout << "Finalizing OpenMC" << std::endl;
  int err = openmc_finalize();
  if (err)
    openmc::fatal_error(openmc_err_msg);

  // ~RayTracingStudy();
}

void OpenMCStudy::generateRays()
{
  std::cout << "/////////// GENERATING NEW RAYS /////////" << std::endl;

  // Create all the rays, place them in _rays
  defineRaysInternal();

  // Assign rays to each process. This is done every time since neutrons keep
  // changing domains. If we do fixed source, we can limit this
  claimRaysInternal();

  //TODO Reinit the elements for the physics

  // Move the rays to the work buffers
  for (auto & ray : _local_rays)
  {
    //TODO All the resetting because generateRays is called everytime
    // ray->resetCounters();
    // ray->clearStartingInfo();

    moveRayToBuffer(ray);
  }
}

void
OpenMCStudy::meshChanged()
{
  RayTracingStudy::meshChanged();

  // Invalidate all of the old starting info because we can't be sure those elements still exist
  for (const auto & ray : _rays)
  {
    ray->invalidateStartingElem();
    ray->invalidateStartingIncomingSide();
  }
  for (const auto & ray : _local_rays)
  {
    ray->invalidateStartingElem();
    ray->invalidateStartingIncomingSide();
  }
}

void
OpenMCStudy::claimRaysInternal()
{
  TIME_SECTION(_claim_rays_timer);
  CONSOLE_TIMED_PRINT("Claiming rays");

  _claim_rays.claim();
}

void
OpenMCStudy::defineRaysInternal()
{
  {
    TIME_SECTION(_define_rays_timer);
    CONSOLE_TIMED_PRINT("Defining rays");

    _rays.clear();
    _local_rays.clear();

    defineRays();
  }

  // Do we actually have Rays
  auto num_rays = _rays.size();
  _communicator.sum(num_rays);
  if (!num_rays)
    mooseError("No Rays were moved to _rays in defineRays()");
  for (const auto & ray : _rays)
    if (!ray)
      mooseError("A nullptr Ray was found in _rays after defineRays().");

}

void
OpenMCStudy::defineRays()
{
  // Initialize total weight and tallies list
  openmc::initialize_batch();
  openmc::initialize_generation();

  // Set the batch and generation number
  //TODO separate batches and generations
  openmc::simulation::current_batch = _t_step;
  openmc::simulation::current_gen = 1;
  openmc::Particle neutron;

  // Loop over particles. create the rays
  // This needs to be done over all processes, since we do not know where the particle will
  // be created.
  //TODO OpenMP parallelism
  for (int64_t i = 0; i < openmc::simulation::source_bank.size(); ++i)
  {
    // Get a ray from the study
    std::shared_ptr<Ray> ray = acquireRay();

    // Have OpenMC initialize all the information
    openmc::initialize_history(neutron, i + 1);

    /* includes :
      position, direction, energy, weight, children
      filter initialization
      various indexes, tally derivatives etc
    */

    // Set starting information
    Point start(neutron.r()[0], neutron.r()[1], neutron.r()[2]);
    Point direction(neutron.u()[0], neutron.u()[1], neutron.u()[2]);

    // Claimer will locate the starting point
    ray->setStart(start);
    ray->setStartingDirection(direction);

    // Store neutron information
    ray->auxData(0) = neutron.E();
    ray->auxData(1) = neutron.wgt();

    _rays.emplace_back(std::move(ray));
  }

}

// void OpenMCStudy::execute()
// {
//   std::cout << "In execute" << std::endl;
//   // This may be needed to handle the batching of neutrons
//
// }

void OpenMCStudy::postExecuteStudy()
{
  // Reduce global tallies, sort and distribute the fission bank
  //TODO: Not much need to distribute the fission bank, with the claiming process
  openmc::finalize_generation();

  // Reduce all tallies, write state/source_point, run CMFD,
  openmc::finalize_batch();
}

void OpenMCStudy::checkOpenMCVersion()
{
  if (openmc::VERSION_MAJOR < 0 || openmc::VERSION_MINOR < 13)
    mooseWarning("OpenMC version detected ", openmc::VERSION_MAJOR, ":", openmc::VERSION_MINOR,
        "is anterior to the supported version (0.13).");
  if (openmc::VERSION_MAJOR > 0 || openmc::VERSION_MINOR > 13)
    mooseWarning("OpenMC version detected ", openmc::VERSION_MAJOR, ":", openmc::VERSION_MINOR,
        "is posterior to the supported version (0.13).");

  //TODO Check for specifically unsupported versions
}
