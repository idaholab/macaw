[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 5
    ny = 5
    nz = 5
    xmin = -5
    ymin = -5
    zmin = -5
    xmax = 5
    ymax = 5
    zmax = 5
  []
  [./add_subdomain]
    input = gmg
    type = SubdomainBoundingBoxGenerator
    top_right = '1 1 1'
    bottom_left = '-1 -1 -1'
    block_id = 1
    block_name = 'center'
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
[]

[RayKernels/collision]
  type = CollisionKernel
  temperature = temperature
  blocks = "0 1 2"
  materials = "1 1 1" # openmc material id
  # verbose = true
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

  hide = 'num_rays'

  [rays]
    type = RayTracingExodus
    study = study
    output_data = true # enable for data output
    # output_data_nodal = true # enable for nodal data output
    output_aux_data = true
    execute_on = final
  []
[]

# To look at domain decomposition
[AuxVariables/domain]
[]

[AuxKernels]
  [domains]
    type = ProcessorIDAux
    variable = domain
  []
[]

# To measure performance
[Postprocessors]
  [total_mem]
    type = MemoryUsage
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [per_proc]
    type = MemoryUsage
    value_type = "average"
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_proc]
    type = MemoryUsage
    value_type = "max_process"
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [total_time]
    type = PerfGraphData
    execute_on = 'INITIAL TIMESTEP_END'
    data_type = 'TOTAL'
    section_name = 'Root'
  []
  [run_time]
    type = ChangeOverTimePostprocessor
    postprocessor = total_time
    execute_on = 'INITIAL TIMESTEP_END'
  []

  # Neutrons per second to compare to openmc
  [num_rays]
    type = VectorPostprocessorComponent
    vectorpostprocessor = per_proc_ray_tracing
    index = 0
    vector_name = rays_traced
  []
  [total_num_rays]
    type = CumulativeValuePostprocessor
    postprocessor = num_rays
  []
  [neutrons_per_s]
    type = ParsedPostprocessor
    pp_names = 'total_num_rays total_time'
    function = 'total_num_rays / total_time'
  []
[]

[VectorPostprocessors/per_proc_ray_tracing]
  type = PerProcessorRayTracingResultsVectorPostprocessor
  execute_on = TIMESTEP_END
  study = study
[]
