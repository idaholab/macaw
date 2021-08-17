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

// For finalizing generations // TODO Make PR to OpenMC finalize_generation
#include "openmc/tallies/tally.h" // TODO Use C-API instead
#include "openmc/constants.h"
#include "openmc/eigenvalue.h"
#include "openmc/output.h"

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
  params.addParam<bool>("verbose", true, "Whether to output the current stage of the simulation");

  // By default, let's not verify Rays in optimized modes because it's so expensive
#ifndef NDEBUG
  params.set<bool>("verify_rays", false);
#endif
  // We don't typically have internal sidesets in Monte Carlo
  // params.set<bool>("use_internal_sidesets") = false;
  // params.suppressParameter<bool>("use_internal_sidesets");
  // particles dont need to be named
  params.addPrivateParam<bool>("_use_ray_registration", false);
  // We manage banking Rays as needed on our own
  params.set<bool>("_bank_rays_on_completion") = false;
  // Subdomain setup does not depend on individual Rays in Monte Carlo
  params.set<bool>("_ray_dependent_subdomain_setup") = false;

  return params;
}

OpenMCStudy::OpenMCStudy(const InputParameters & params)
  : RayTracingStudy(params),
    _rays(declareRestartableDataWithContext<std::vector<std::shared_ptr<Ray>>>("rays", this)),
    _local_rays(
        declareRestartableDataWithContext<std::vector<std::shared_ptr<Ray>>>("local_rays", this)),
    _claim_rays(*this, _rays, _local_rays, true),
    _claim_rays_timer(registerTimedSection("claimRays", 1)),
    _define_rays_timer(registerTimedSection("defineRays", 1)),
    _is_2D(_mesh.dimension() == 2),
    _verbose(getParam<bool>("verbose"))
{
  /*
  This class doesnt really need to get anything that can be in OpenMC input files:
  - materials
  - settings (number of particles, batches, etc)
  - even tallies (for now, we'll just replicated)!

  Its real objective should be to call each part of openmc_run, and replace just the core stuff with
  'Ray' instead of particles. Maybe we could even make a subclass of 'Particle, particle', that also
  inherits from 'Ray' so that both OpenMC and MOOSE know what to do with it

  There are a few transfers we want to make:
  - temperature, MAYBE. It could be handled on moose side! (and sent to OpenMC via Particle T
  attribute)
  - power distribution tally, from an OpenMC tally to MOOSE

  LONG TERM:
  - moose need to handle the tallies, because we need domain decomposition
  - more than 1 batch ? If it's annoying

  How to handle OpenMC physics calls
  // save the OpenMC particle in AuxData
  // OR
  // make a class that inherits from both
  // OR
  // use fake particles to call the routines and move results over
  // OR
  // use fake particles created once and use references to move results over
  */

  if (_verbose)
    _console << "Initializing OpenMC simulation" << std::endl;

  // Initialize run
  char * argv[1] = {(char *)"openmc"};
  int err = openmc_init(1, argv, &_communicator.get());
  if (err)
    openmc::fatal_error(openmc_err_msg);

  // Check run mode
  if (openmc::settings::run_mode == openmc::RunMode::PARTICLE ||
      openmc::settings::run_mode == openmc::RunMode::PLOTTING ||
      openmc::settings::run_mode == openmc::RunMode::VOLUME)
    mooseError("Requested run mode is currently not supported by MaCaw");

  openmc_simulation_init();
  // does data OK
  // work TODO -> no need here
  // banks TODO -> no need for us
  // tally initialization TODO
  // nuclide index mapping OK

  // Set size of source bank for the first batch
  _source_bank_size = openmc::simulation::work_per_rank;

  // Single level tree to represent block containing cells
  openmc::model::n_coord_levels = 1;

  // TODO Figure out how to distribute that
  // TODO Use active elements, for adaptivity

  // Resize the number of cells in openmc to the number of elements in moose
  openmc::model::cells.resize(_mesh.getMesh().n_active_local_elem());
  openmc::model::cell_map.clear();

  // Resize the number of universes in openmc to the number of blocks in moose
  openmc::model::universes.resize(_mesh.getMesh().n_subdomains());
  openmc::model::universe_map.clear();

  int i = -1;
  for (const auto & elem : *mesh().getActiveLocalElementRange())
  {
    i++;
    const auto & elem_id = elem->id();
    const auto & univ_id = elem->subdomain_id();

    // Create cells for each element of the domain and assign their universe to the block
    openmc::model::cells[i] = gsl::make_unique<openmc::CSGCell>();
    openmc::model::cells[i]->id_ = elem_id;
    openmc::model::cells[i]->universe_ = univ_id;
    openmc::model::cell_map[elem_id] = elem_id;

    // Add all universes to the universe map, and keep track of the cells in each universe
    auto it = openmc::model::universe_map.find(univ_id);
    if (it == openmc::model::universe_map.end())
    {
      openmc::model::universes.push_back(gsl::make_unique<openmc::Universe>());
      openmc::model::universes.back()->id_ = univ_id;
      openmc::model::universes.back()->cells_.push_back(elem_id);
      openmc::model::universe_map[univ_id] = openmc::model::universes.size() - 1;
    }
    else
      openmc::model::universes[it->second]->cells_.push_back(elem_id);
  }

  // TODO Initialize this nicer
  registerRayAuxData("energy");
  registerRayAuxData("weight");
  registerRayAuxData("n_progeny");
  registerRayAuxData("id");
  registerRayAuxData("seed_tracking");
  registerRayAuxData("seed_source");
  registerRayAuxData("seed_URR");
  registerRayAuxData("particle_type");

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
  if (_verbose)
    _console << "Finalizing OpenMC" << std::endl;

  int err = openmc_finalize();
  if (err)
    openmc::fatal_error(openmc_err_msg);
}

void
OpenMCStudy::generateRays()
{
  if (_verbose)
    _console << "Generating new rays" << std::endl;

  // Create all the rays, place them in _rays
  defineRaysInternal();

  // Assign rays to each process. This is done every time since particles keep
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
  if (_verbose)
    _console << "Adapting to mesh changes" << std::endl;

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
  if (_verbose)
    _console << "Defining " << _source_bank_size << " rays" << std::endl;

  // Initialize total weight, tallies list, reset fission bank
  openmc::initialize_batch();
  openmc::initialize_generation();

  // Set the batch and generation number
  //TODO separate batches and generations
  openmc::simulation::current_batch = _t_step;
  openmc::simulation::current_gen = 1;
  openmc::Particle p;

  // Loop over particles. create the rays
  // This needs to be done over all processes, since we do not know where the particle will
  // be created.
  //TODO OpenMP parallelism
  for (int64_t i = 0; i < _source_bank_size; ++i)
  {
    // Get a ray from the study
    std::shared_ptr<Ray> ray = acquireRay();

    // Temporary print to understand why rays arent fully re-used
    // _console << ray->getInfo() << std::endl;

    // Have OpenMC initialize all the information
    openmc::initialize_history(p, i + 1);

    /* includes :
      position, direction, energy, weight, children
      filter initialization
      various indexes, tally derivatives etc
    */

    // Store p information
    ray->auxData(0) = p.E();
    ray->auxData(1) = p.wgt();

    // Reset number of progeny particles
    ray->auxData(2) = 0;

    // Keep track of openmc particle id
    ray->auxData(3) = p.id();

    // Keep track of the particle seed for consistent random number generation
    ray->auxData(4) = p.seeds(0);
    ray->auxData(5) = p.seeds(1);
    ray->auxData(6) = p.seeds(2);

    // Keep track of particle type
    ray->auxData(7) = int(p.type());

    // Set starting information
    Point start(p.r()[0], p.r()[1], p.r()[2]);
    Point direction(p.u()[0], p.u()[1], p.u()[2]);
    if (_is_2D)
      direction(2) = 0;

    // Claimer will locate the starting point
    ray->setStart(start);
    ray->setStartingDirection(direction);

    _rays.emplace_back(std::move(ray));
  }
}

void
OpenMCStudy::execute()
{
  if (_verbose)
    _console << "Executing study" << std::endl;
  RayTracingStudy::execute();
}

void
OpenMCStudy::postExecuteStudy()
{
  if (_verbose)
    _console << "Finalizing generations and batches" << std::endl;

  // Reduce global tallies, sort and distribute the fission bank
  if (comm().size() <= 1)
    openmc::finalize_generation();
  else
  {
    // For domain decomposed Monte Carlo, we cannot sort sites using OpenMC's sort_fission_bank
    // and we cannot synchronize the bank using OpenMC's synchronize_bank because some domains
    // may have sampled 0 sites
    finalizeGeneration();
  }

  // Stop openmc from reducing tallies (arrays are not the same size)
  openmc::mpi::n_procs = 1;

  // Reduce all tallies, write state/source_point, run CMFD
  openmc::finalize_batch();
  openmc::mpi::n_procs = comm().size();
}

void
OpenMCStudy::finalizeGeneration()
{
  auto & gt = openmc::simulation::global_tallies;

  // Update global tallies with the accumulation variables
  if (openmc::settings::run_mode == openmc::RunMode::EIGENVALUE)
  {
    gt(openmc::GlobalTally::K_COLLISION, openmc::TallyResult::VALUE) +=
        openmc::global_tally_collision;
    gt(openmc::GlobalTally::K_ABSORPTION, openmc::TallyResult::VALUE) +=
        openmc::global_tally_absorption;
    gt(openmc::GlobalTally::K_TRACKLENGTH, openmc::TallyResult::VALUE) +=
        openmc::global_tally_tracklength;
  }
  gt(openmc::GlobalTally::LEAKAGE, openmc::TallyResult::VALUE) += openmc::global_tally_leakage;

  // reset tallies
  if (openmc::settings::run_mode == openmc::RunMode::EIGENVALUE)
  {
    openmc::global_tally_collision = 0.0;
    openmc::global_tally_absorption = 0.0;
    openmc::global_tally_tracklength = 0.0;
  }
  openmc::global_tally_leakage = 0.0;

  if (openmc::settings::run_mode == openmc::RunMode::EIGENVALUE)
  {

    // TODO Sort bank

    // Synchronize all fission banks to keep the same number of starting rays every batch
    synchronizeBanks();

    // Calculate shannon entropy
    if (openmc::settings::entropy_on)
      openmc::shannon_entropy();

    // Collect results and statistics
    openmc::calculate_generation_keff();
    openmc::calculate_average_keff();

    // Write generation output
    if (comm().rank() == 0 && openmc::settings::verbosity >= 7)
    {
      openmc::print_generation();
    }
  }
}

void
OpenMCStudy::synchronizeBanks()
{
  if (_verbose)
    _console << "Synchronizing fission bank" << std::endl;

  /* Adapted from OpenMC */
  // ==========================================================================
  // Compute total number of fission sites sampled
  int64_t start = 0;
  int64_t n_bank = openmc::simulation::fission_bank.size();
  MPI_Exscan(&n_bank, &start, 1, MPI_INT64_T, MPI_SUM, comm().get());

  // While we would expect the value of start on rank 0 to be 0, the MPI
  // standard says that the receive buffer on rank 0 is undefined and not
  // significant
  if (comm().rank() == 0)
    start = 0;

  int64_t finish = start + openmc::simulation::fission_bank.size();
  int64_t total = finish;
  MPI_Bcast(&total, 1, MPI_INT64_T, comm().size() - 1, comm().get());

  // Compute sampling probability
  int64_t id = openmc::simulation::total_gen + openmc::overall_generation();
  uint64_t seed = openmc::init_seed(id, openmc::STREAM_TRACKING);
  openmc::advance_prn_seed(start, &seed);

  // Determine how many fission sites we need to sample from the source bank
  // and the probability for selecting a site.

  int64_t sites_needed;
  if (total < openmc::settings::n_particles)
  {
    sites_needed = openmc::settings::n_particles % total;
  }
  else
  {
    sites_needed = openmc::settings::n_particles;
  }
  double p_sample = static_cast<double>(sites_needed) / total;

  if (_verbose)
  {
    if (comm().rank() == 0)
      _console << "  Total fission source sites sampled: " << total << std::endl;
    _console << "    Sampled on rank " << comm().rank() << " : " << n_bank << " -> finish "
             << finish << std::endl;
  }
  if (total == 0)
    mooseError("No fission sites sampled on any process, cannot sample a new batch");

  // ==========================================================================
  // SAMPLE N_PARTICLES FROM FISSION BANK AND PLACE IN TEMP_SITES

  // Allocate temporary source bank -- we don't really know how many fission
  // sites were created, so overallocate
  int64_t index_temp = 0;
  std::vector<openmc::SourceSite> temp_sites;
  temp_sites.resize(openmc::simulation::fission_bank.size() *
                    std::max(1, int(openmc::settings::n_particles / total) + 1));

  for (int64_t i = 0; i < openmc::simulation::fission_bank.size(); i++)
  {
    const auto & site = openmc::simulation::fission_bank[i];

    // If there are less than n_particles particles banked, automatically add
    // int(n_particles/total) sites to temp_sites. For example, if you need
    // 1000 and 300 were banked, this would add 3 source sites per banked site
    // and the remaining 100 would be randomly sampled.
    if (total < openmc::settings::n_particles)
    {
      for (int64_t j = 1; j <= openmc::settings::n_particles / total; ++j)
      {
        temp_sites[index_temp] = site;
        ++index_temp;
      }
    }

    // Randomly sample sites needed
    if (openmc::prn(&seed) < p_sample)
    {
      temp_sites[index_temp] = site;
      ++index_temp;
    }
  }

  // ==========================================================================
  // Update start and finish indexes

  // First do an exclusive scan to get the starting indices for
  start = 0;
  MPI_Exscan(&index_temp, &start, 1, MPI_INT64_T, MPI_SUM, comm().get());
  finish = start + index_temp;
  _console << "    New finish rank " << comm().rank() << " : " << finish << std::endl;

  // ==========================================================================
  // Now that the sampling is complete, we need to ensure that we have exactly
  // n_particles source sites. The way this is done in a reproducible manner is
  // to adjust only the source sites on the last processor with active fission
  // sites
  int last_finish;
  int local_finish = finish * (openmc::simulation::fission_bank.size() > 0);
  MPI_Allreduce(&local_finish, &last_finish, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

  if (local_finish == last_finish)
  {
    if (finish > openmc::settings::n_particles)
    {
      // If we have extra sites sampled, we will simply discard the extra
      // ones on the last processor
      index_temp = openmc::settings::n_particles - start;
      if (_verbose)
        _console << "Cutting too long fission source on last active rank" << std::endl;
    }
    else if (finish < openmc::settings::n_particles)
    {
      if (_verbose)
        _console << "Expanding too short fission source on last active rank : "
                 << openmc::settings::n_particles - finish << std::endl;

      // If we have too few sites, repeat sites from the very end of the
      // fission bank
      sites_needed = openmc::settings::n_particles - finish;
      temp_sites.resize(temp_sites.size() + sites_needed);
      for (int i = 0; i < sites_needed; ++i)
      {
        // For low numbers of neutrons, this could go below 0
        int i_bank = std::max(0, int(openmc::simulation::fission_bank.size() - sites_needed + i));
        temp_sites[index_temp] = openmc::simulation::fission_bank[i_bank];
        ++index_temp;
      }
    }
  }
  _console << "Size of temporary bank on rank " << comm().rank() << " : " << index_temp
           << std::endl;

  // Check size of bank before moving sites
  if (openmc::simulation::source_bank.size() < (unsigned)index_temp)
    openmc::simulation::source_bank.resize(index_temp);

  // Move fission sites from temporary array to source bank array
  std::copy(
      temp_sites.begin(), temp_sites.begin() + index_temp, openmc::simulation::source_bank.begin());

  // Keep track of source bank size
  _source_bank_size = index_temp;
  if (index_temp < 0)
    mooseError("Source bank has negative size ", _source_bank_size);

  /* End adapted from OpenMC */
}

void
OpenMCStudy::checkOpenMCVersion()
{
  if (openmc::VERSION_MAJOR < 0 || openmc::VERSION_MINOR < 13)
    mooseWarning("OpenMC version detected ", openmc::VERSION_MAJOR, ":", openmc::VERSION_MINOR,
        "is anterior to the supported version (0.13).");
  if (openmc::VERSION_MAJOR > 0 || openmc::VERSION_MINOR > 13)
    mooseWarning("OpenMC version detected ", openmc::VERSION_MAJOR, ":", openmc::VERSION_MINOR,
        "is posterior to the supported version (0.13).");

  //TODO Check for specifically unsupported versions
}
