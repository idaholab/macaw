# ==============================================================================
# GEOMETRY AND MESH
# ==============================================================================

[Mesh]
  [core]
    type = FileMeshGenerator
    file = 'quarter_core_2d.e'
  []
  [boundaries1]
    type = SideSetsAroundSubdomainGenerator
    input = 'core'
    block = 10016
    normal = ' 1  0  0'
    new_boundary = 'right '
  []
  [boundaries2]
    type = SideSetsAroundSubdomainGenerator
    input = 'boundaries1'
    block = 10016
    normal = '-1  0  0'
    new_boundary = 'left'
  []
  [boundaries3]
    type = SideSetsAroundSubdomainGenerator
    input = 'boundaries2'
    block = 10016
    normal = '0  1  0'
    new_boundary = 'front'
  []
  [boundaries4]
    type = SideSetsAroundSubdomainGenerator
    input = 'boundaries3'
    block = 10016
    normal = '0 -1  0'
    new_boundary = 'back'
  []
  [boundaries5]
    type = SideSetsAroundSubdomainGenerator
    input = 'boundaries4'
    block = 10016
    normal = '0  0  1'
    new_boundary = 'top'
  []
  [boundaries6]
    type = SideSetsAroundSubdomainGenerator
    input = 'boundaries5'
    block = 10016
    normal = '0  0  -1'
    new_boundary = 'bottom'
  []
  [shift_z]
    type = TransformGenerator
    input = 'boundaries6'
    transform = 'TRANSLATE'
    vector_value = '0 0 100'
  []

  [Partitioner]
    type = HierarchicalGridPartitioner
    nx_nodes = 2
    ny_nodes = 2
    nz_nodes = 1

    # 48 processors per node on sawtooth
    nx_procs = 2
    ny_procs = 1
    nz_procs = 1
  []

  # Shared memory run
  # [Partitioner]
  #    type = GridPartitioner
  #    nx = 4
  #    ny = 1
  #    nz = 1
  #  []
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
  [temperature]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = 300
  []
  [power]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[RayKernels/collision]
  type = CollisionKernel
  temperature = temperature
  # mesh block ids
  blocks = "10000 10001 10004 10006 10008 10009 10010 10013 10014"
  # blocks = "1 2 3 4 5 6 7 8 9"
  # openmc material id
  materials = "1 2 5 7 9 10 11 14 15"
  # materials = "14 17 1 7 9 10 11 16 15"
  # verbose = true
  z_coord = 100
[]

[RayBCs]
  [reflect]
    type = ReflectRayBC
    boundary = '3 4' #'back front top right left bottom'
  []
  [vacuum]
    type = KillRayBC
    boundary = '1 2' #'bottom top right front'
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

    # Parameters to make it work for now
    tolerate_failure = true
  []
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
  csv = true
  perf_graph = true
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

[AuxKernels]
  [cell_val]
    type = OpenMCTallyAux
    tally_id = 6
    execute_on = TIMESTEP_END
    variable = power
  []
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

[MultiApps]
  [pin_mesh]
    type = TransientMultiApp
    # execute_on = TIMESTEP_END
    input_files = 'output_pin.i'
  []
[]

[Transfers]
  [power_uo]
    type = MultiAppUserObjectTransfer
    multi_app = pin_mesh
    direction = 'TO_MULTIAPP'
    user_object = pin_powers
    variable = pin_power
    execute_on = TIMESTEP_END
  []
[]

# ==============================================================================
# SIMULATION PERFORMANCE STUDY
# ==============================================================================

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
