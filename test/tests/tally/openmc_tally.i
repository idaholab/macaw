[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 5
    ny = 5
    nz = 1
    xmin = -2.5
    ymin = -2.5
    zmin = -2.5
    xmax = 2.5
    ymax = 2.5
    zmax = 2.5
  []
  # [add_subdomain]
  #   input = gmg
  #   type = SubdomainBoundingBoxGenerator
  #   top_right = '0.5 0.5 2.5'
  #   bottom_left = '-0.5 -0.5 -2.5'
  #   block_id = 1
  # []
[]

[Problem]
  solve = false

  kernel_coverage_check = false
[]

# Main things we care about for the coupling
[Variables/temperature]
  initial_condition = 300
[]

[AuxVariables/power]
  order = CONSTANT
  family = MONOMIAL
[]

[RayKernels/collision]
  type = CollisionKernel
  temperature = temperature
  # mesh block ids
  blocks = "0"
  # openmc material id
  materials = "1"
  # verbose = true
[]

[RayBCs]
  [reflect]
    type = ReflectRayBC
    boundary = 'back front top right left bottom'
  []
[]

[UserObjects]
  inactive = 'univtally'

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
    scores = 'fission scatter'
    filters = 'cell energy'
    energy_bins = '1e-5 1 1e7'
    nuclides = 'U235 O16 U238'
    execute_on = 'initial'
  []
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
  csv = true
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
    granularity = cell
    estimator = COLLISION
    particle_type = neutron
    execute_on = TIMESTEP_END
    variable = power
    nuclide = 'U238'
    score = 'scatter'
  []
[]
