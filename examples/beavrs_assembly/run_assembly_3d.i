[Mesh]
  [assembly]
    type = FileMeshGenerator
    file = '31enr_16_in.e'
  []
  [translate]
    type = TransformGenerator
    input = assembly
    transform = TRANSLATE
    vector_value = '0 0 -100'
  []
  [extrude]
    type = FancyExtruderGenerator
    input = 'translate'
    heights = '20 15 1.748 0.4141 3.3579 57.505 5.715 46.482 5.715 46.482 5.715 46.482 5.715 46.482 5.715 46.482 5.715 37.783 9.298 3.358 2 2.54 3.345 8.827 28.124'
    # num_layers = '1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1'  # 25 zones
    # num_layers = '2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2'  # 25 zones
    num_layers = '10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10'  # 25 zones
    direction = '0 0 1'
    bottom_sideset = '8'
    top_sideset = '9'
  []
  [boundaries1]
    type = SideSetsAroundSubdomainGenerator
    input = 'extrude'
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
  # [shift_z]
  #   type = TransformGenerator
  #   input = 'boundaries6'
  #   transform = 'TRANSLATE'
  #   vector_value = '0 0 100'
  # []
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
    boundary = '1 2 3 4 8 9' #'back front top right left bottom'
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

    # Extruding does lead to ugly meshes
    warn_subdomain_hmax = false
    warn_non_planar = false
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
    execute_on = 'initial'
  []

  [celltally]
    type = OpenMCTally
    id = 1
    particle_type = 'neutron'
    estimator = 'COLLISION'
    scores = 'kappa-fission'
    filters = 'cell'
    execute_on = 'initial'
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

# To look at domain decomposition
[AuxVariables/domains]
[]

[AuxKernels]
  [domains]
    type = ProcessorIDAux
    variable = domains
  []

  [cell_val]
    type = OpenMCTallyAux
    granularity = 'cell'
    score = 'kappa-fission'
    tally_id = 1
    execute_on = TIMESTEP_END
    variable = power
  []
[]

# To tally reaction rates on a pincell mesh
[VectorPostprocessors]
  [pin_powers]
    type = NearestPointIntegralVariablePostprocessor
    variable = 'power'
    block = '10010'
    points_file = pin_file
  []
[]

[MultiApps]
  [pin_mesh]
    type = TransientMultiApp
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
