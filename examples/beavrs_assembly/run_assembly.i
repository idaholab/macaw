[Mesh]
  [core]
    type = FileMeshGenerator
    file = '31enr_16_in.e'
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
[]

[Problem]
  solve = false
  kernel_coverage_check = false
  skip_nl_system_check = true
[]

# Main things we care about for the coupling
[AuxVariables]
  [temperature]
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
  blocks = "10000 10001 10004 10006 10008 10009 10010 10013 10014 11006"
  # blocks = "1 2 3 4 5 6 7 8 9"
  # openmc material id
  materials = "1 2 5 7 9 10 11 14 15 7"
  # materials = "14 17 1 7 9 10 11 16 15"
  # verbose = true
  z_coord = 100
[]

[RayBCs]
  [reflect]
    type = ReflectRayBC
    boundary = '1 2 3 4' #'back front top right left bottom'
  []
[]

[UserObjects]
  inactive = 'tally univtally'

  [study]
    type = OpenMCStudy

    execute_on = TIMESTEP_END

    # Needed to cache trace information for RayTracingMeshOutput
    always_cache_traces = true
    segments_on_cache_traces = true

    # Needed to cache Ray data for RayTracingMeshOutput
    data_on_cache_traces = true
    aux_data_on_cache_traces = true
  []

  [tally]
    type = OpenMCTally
    particle_type = 'neutron'
    estimator = 'COLLISION'
    scores = 'flux scatter (n,fission) 16'
    filters = 'energy particle'
    energy_bins = '1e-5 1e3 2e7'
    execute_on = 'initial'
  []

  [univtally]
    type = OpenMCTally
    particle_type = 'neutron'
    estimator = 'COLLISION'
    scores = 'kappa-fission'
    filters = 'universe'
    filter_ids = 1
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

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = false
  csv = true
  perf_graph = true
[]

# To look at domain decomposition
[AuxVariables/domains]
[]

[AuxKernels]
  [domains]
    type = ProcessorIDAux
    variable = domains
  []

  # [cell_val]
  #   type = OpenMCTallyAux
  #   tally_id = 6
  #   execute_on = TIMESTEP_END
  #   variable = power
  # []
[]
