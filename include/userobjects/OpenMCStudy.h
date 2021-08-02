//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "RayTracingStudy.h"
#include "ClaimRays.h"

// openmc includes

// TODO: Consider inheriting from RepeatableRayStudyBase

/**
 * Class to use the Monte Carlo method for neutron transport. Wraps OpenMC function
 * calls for initializing simulation, for initialization and banking of neutrons
 */
class OpenMCStudy : public RayTracingStudy
{
public:
  OpenMCStudy(const InputParameters & parameters);
  ~OpenMCStudy();

  static InputParameters validParams();

protected:
private:
  void meshChanged() override;
  void generateRays() override;
  void execute() override;
  void postExecuteStudy() override;

  // TODO Delete if they only provide a timer
  void claimRaysInternal();
  void defineRaysInternal();

  void defineRays();

  /// Routine to collect tallies, and void fission bank sourcing and synchronizing
  void finalizeGeneration();

  /// Routine to synchronize banks across all processors
  void synchronizeBanks();

  /// Routine to warn about potential incompatibility issues
  void checkOpenMCVersion();

  /// Vector of Rays that the user will fill into in defineRays() (restartable)
  std::vector<std::shared_ptr<Ray>> & _rays;

  /// Storage for all of the Rays this processor is responsible for (restartable)
  std::vector<std::shared_ptr<Ray>> & _local_rays;

  /// The object used to claim Rays
  ClaimRays _claim_rays;

  /// Timing for claiming rays
  PerfID _claim_rays_timer;
  /// Timing for defining rays
  PerfID _define_rays_timer;

  // The size of the particle source bank on this process
  unsigned int _source_bank_size;

  // Whether to output which stage of the simulation the solver is going through
  bool _verbose;
};
