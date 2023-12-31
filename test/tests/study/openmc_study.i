[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 5
    ny = 5
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
    top_right = '1 1 1'
    bottom_left = '-1 -1 -1'
    block_id = 1
    block_name = 'center'
  []
[]

[Problem]
  solve = false

  kernel_coverage_check = false
[]

# Main things we care about for the coupling
[Variables/temperature]
    order = CONSTANT
    family = MONOMIAL
[]

[AuxVariables/power]
    order = CONSTANT
    family = MONOMIAL
[]

[RayKernels/collision]
  type = CollisionKernel
  temperature = temperature
  blocks = "0 1 2"
  materials = "1 2 3"  # openmc material id minus one !
[]

[RayBCs]
  [reflect]
    type = ReflectRayBC
    boundary = 'back front top right left bottom'
  []
[]

[UserObjects/study]
  type = OpenMCStudy

  execute_on = TIMESTEP_END

  # Needed to cache trace information for RayTracingMeshOutput
  always_cache_traces = true
  segments_on_cache_traces = true

  # Needed to cache Ray data for RayTracingMeshOutput
  data_on_cache_traces = true
  aux_data_on_cache_traces = true
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = false
  csv = true
  [rays]
    type = RayTracingExodus
    study = study
    output_data = true # enable for data output
    output_aux_data = true
    execute_on = final
  []
[]
