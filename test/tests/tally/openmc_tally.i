[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 10
    ny = 10
    nz = 1
    xmin = -5
    ymin = -5
    zmin = -5
    xmax = 5
    ymax = 5
    zmax = 5
  []
  [add_subdomain]
    input = gmg
    type = SubdomainBoundingBoxGenerator
    top_right = '3 3 5'
    bottom_left = '-3 -3 -5'
    block_id = 1
  []
[]

[Problem]
  solve = false

  kernel_coverage_check = false
[]

# Main things we care about for the coupling
[Variables/temperature]
  initial_condition = 300
[]

[AuxVariables/tally]
  order = CONSTANT
  family = MONOMIAL
[]

[RayKernels/collision]
  type = CollisionKernel
  temperature = temperature
  # mesh block ids
  blocks = "0 1"
  # openmc material id
  materials = "4 1"
  # verbose = true
[]

[RayBCs]
  [reflect]
    type = ReflectRayBC
    boundary = 'back front top right left bottom'
  []
[]

[UserObjects]
  active = 'study celltally'

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



  [univtally]
    type = OpenMCTally
    id = 1
    particle_type = 'neutron'
    estimator = 'COLLISION'
    scores = 'flux fission'
    filters = 'universe'
    execute_on = 'initial'
  []

  [celltally]
    type = OpenMCTally
    id = 1
    particle_type = 'neutron'
    estimator = 'COLLISION'
    scores = 'flux fission'
    filters = 'cell'
    # energy_bins = '1e-5 1e5 1e7'
    # nuclides = 'U235 O16 U238'
    execute_on = 'initial'
  []
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
  csv = true
  # [rays]
  #   type = RayTracingExodus
  #   study = study
  #   output_data = true # enable for data output
  #   # output_data_nodal = true # enable for nodal data output
  #   output_aux_data = true
  #   execute_on = final
  # []
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
    tally_id = 1
    granularity = universe
    estimator = COLLISION
    particle_type = neutron
    execute_on = TIMESTEP_END
    variable = tally
    # nuclide = 'U238'
    score = 'fission'
    # energy_bin = 0
  []
[]
