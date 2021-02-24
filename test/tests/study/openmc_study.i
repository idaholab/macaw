[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    xmax = 5
    ymax = 5
  []
[]

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

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
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

[UserObjects/study]
  type = OpenMCStudy

  execute_on = TIMESTEP_END

  # Needed to cache trace information for RayTracingMeshOutput
  always_cache_traces = true
  # Needed to cache Ray data for RayTracingMeshOutput
  data_on_cache_traces = true
  aux_data_on_cache_traces = true
[]

[RayKernels/u_integral]
  type = VariableIntegralRayKernel
  variable = u
  rays = 'diag right_up'
[]

[Postprocessors]
  [diag_line_integral]
    type = RayIntegralValue
    ray_kernel = u_integral
    ray = diag
  []
  [right_up_line_integral]
    type = RayIntegralValue
    ray_kernel = u_integral
    ray = right_up
  []
[]
