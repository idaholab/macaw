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
[]

[Problem]
  solve = false
[]

#############
# mock diffusion problem to avoid the coverage checks
[Variables/u]
[]

[Kernels/diff]
  type = Diffusion
  variable = u
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]
#############

[RayKernels/collision]
  type = CollisionKernel
  temperature = u
  blocks = "0 1 2"
  materials = "1 2 3"
[]
[RayKernels/u_integral]
  type = VariableIntegralRayKernel
  variable = u
  # rays = 'diag right_up'
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
    # output_data_nodal = true # enable for nodal data output
    output_aux_data = true
    execute_on = final
  []
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
[]
