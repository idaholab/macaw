# ==============================================================================
# GEOMETRY AND MESH
# ==============================================================================

[Mesh]
  # Load the full 2D mesh
  [core]
    type = FileMeshGenerator
    file = 'full.cpr'
  []

  # Extrude it
  [extrude]
    type = FancyExtruderGenerator
    input = core
    heights = '20 15 1.748 0.4141 3.3579 57.505 5.715 46.482 5.715 46.482 5.715 46.482 5.715 46.482 5.715 46.482 5.715 37.783 9.298 3.358 2 2.54 3.345 8.827 28.124'
    num_layers = '1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1'  # 25 zones
    direction = '0 0 1'
    bottom_boundary = '100'
    top_boundary = '101'
  []

  # Repartition mesh on all processes
  # Full MPI run
  # [Partitioner]
  #   type = HierarchicalGridPartitioner
  #   nx_nodes = 5
  #   ny_nodes = 5
  #   nz_nodes = 4
  #
  #   # 48 processors per node on sawtooth
  #   nx_procs = 4
  #   ny_procs = 4
  #   nz_procs = 3
  # []

  # Shared memory run
  [Partitioner]
     type = GridPartitioner
     nx = 5
     ny = 5
     nz = 4
   []
[]

# ==============================================================================
# SET UP OPENMC SIMULATION IN MOOSE
# ==============================================================================

[Problem]
  solve = false
  kernel_coverage_check = false
  skip_nl_system_check = true
[]

# Main things we care about for the coupling
[AuxVariables]
  # [temperature]
  #   order = CONSTANT
  #   family = MONOMIAL
  #   initial_condition = 300
  # []
[]

[RayKernels/collision]
  type = CollisionKernel
  temperature = 300 #temperature
  # mesh block ids
  blocks = "10000 10001 10004 10006 10008 10009 10010 10013 10014"
  # blocks = "1 2 3 4 5 6 7 8 9"

  # openmc material id
  materials = "1 2 5 7 9 10 11 14 15"
  # materials = "14 17 1 7 9 10 11 16 15"

  # verbose = true
  # z_coord = 100
[]

[RayBCs]
  [vacuum]
    type = KillRayBC
    boundary = 'top right left bottom 100 101'
  []
[]

[UserObjects]

  [study]
    type = OpenMCStudy

    execute_on = TIMESTEP_END

    # Needed to cache trace information for RayTracingMeshOutput
    always_cache_traces = false
    segments_on_cache_traces = false

    # Needed to cache Ray data for RayTracingMeshOutput
    data_on_cache_traces = false
    aux_data_on_cache_traces = false

    # Extruding does lead to ugly meshes
    warn_subdomain_hmax = false
    warn_non_planar = false

    # Parameters to make it work for now
    tolerate_failure = true
  []
[]

[Executioner]
  type = Transient
[]

# ==============================================================================
# TALLIES
# ==============================================================================

[UserObjects]
  [tally]
    type = OpenMCTally
    id = 3
    particle_type = 'neutron'
    estimator = 'COLLISION'
    scores = 'flux scatter (n,fission) 16'
    filters = 'energy particle'
    energy_bins = '1e-5 1e3 2e7'
    filter_ids = 3
    execute_on = 'initial'
  []

  [univtally]
    type = OpenMCTally
    id = 2
    particle_type = 'neutron'
    estimator = 'COLLISION'
    scores = 'kappa-fission'
    filters = 'universe'
    filter_ids = 2
    execute_on = 'initial'
  []

  [celltally]
    type = OpenMCTally
    id = 1
    particle_type = 'neutron'
    estimator = 'COLLISION'
    scores = 'kappa-fission'
    filters = 'cell'
    filter_ids = 1
    execute_on = 'initial'
  []
[]

[AuxVariables]
  [power]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  # [cell_val]
  #   type = OpenMCTallyAux
  #   tally_id = 6
  #   execute_on = TIMESTEP_END
  #   variable = power
  # []
[]

# Output on a pincell mesh
[VectorPostprocessors]
  [pin_powers]
    type = NearestPointIntegralVariablePostprocessor
    variable = 'power'
    block = '10008 10009 10010'
    points_file = pin_file
  []
[]

# [MultiApps]
#   [pin_mesh]
#     type = TransientMultiApp
#     # execute_on = TIMESTEP_END
#     input_files = 'output_pin.i'
#   []
# []
#
# [Transfers]
#   [power_uo]
#     type = MultiAppUserObjectTransfer
#     multi_app = pin_mesh
#     direction = 'TO_MULTIAPP'
#     user_object = pin_powers
#     variable = pin_power
#     execute_on = TIMESTEP_END
#   []
# []


# ==============================================================================
# SIMULATION PERFORMANCE STUDY
# ==============================================================================

[Outputs]
  exodus = false
  nemesis = true
  csv = true
  perf_graph = true
[]

# To look at domain decomposition
[AuxVariables/domains]
  order = CONSTANT
  family = MONOMIAL
[]

[AuxKernels]
  [domains]
    type = ProcessorIDAux
    variable = domains
  []
[]

[VectorPostprocessors]
  [mem]
    type = VectorMemoryUsage
    execute_on = 'INITIAL TIMESTEP_END'
    report_peak_value = true
    mem_units = megabytes
  []
[]
