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
  [./add_subdomain]
    input = gmg
    type = SubdomainBoundingBoxGenerator
    top_right = '0.5 0.5 2.5'
    bottom_left = '-0.5 -0.5 -2.5'
    block_id = 1
    #block_name = 'center'
  [../]
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
  blocks = "0 1 2"
  materials = "0 3 2"  # openmc material id minus one !
  verbose = true
[]
[RayKernels/u_integral]
  type = VariableIntegralRayKernel
  variable = temperature
  # rays = 'diag right_up'
[]

[RayBCs]
  [reflect]
    type = ReflectRayBC
    boundary = 'back front top right left bottom'
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
    tally_estimator = 'COLLISION'
    tally_scores = 'flux scatter (n,fission) 16'
    tally_filters = 'energy particle'
    tally_energy_bins = '1e-5 1e3 2e7'

    execute_on = 'initial'
  []

  [univtally]
    type = OpenMCTally


    particle_type = 'neutron'
    tally_estimator = 'COLLISION'
    tally_scores = 'kappa-fission'
    tally_filters = 'universe'
    filter_ids = 1
    execute_on = 'initial'
  []

  [celltally]
    type = OpenMCTally
    tally_id = 1
    particle_type = 'neutron'
    tally_estimator = 'COLLISION'
    tally_scores = 'flux'
    tally_filters = 'cell'
    filter_ids = 1
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
  # inactive = 'test_aux'
  [domains]
    type = ProcessorIDAux
    variable = domains
  []

  [cell_tally_val]
    type = OpenMCTallyAux
    tally_id = 1
    execute_on = TIMESTEP_END
    variable = power

  []

  # [test_aux]
  #   type = ConstantAux
  #   variable = power
  #   value = 5
  # []
[]

[Postprocessors]
  # [diag_line_integral]
  #   type = RayIntegralValue
  #   ray_kernel = u_integral
  #   # ray = diag
  # []
  # [right_up_line_integral]
  #   type = RayIntegralValue
  #   ray_kernel = u_integral
  #   # ray = right_up
  # []
  # [avgval]
  #   type = ElementAverageValue
  #   variable = power
  #   execute_on = FINAL
  # []
[]
