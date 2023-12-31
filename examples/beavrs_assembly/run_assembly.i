# ==============================================================================
# GEOMETRY AND MESH
# ==============================================================================

[Mesh]
  [core]
    type = FileMeshGenerator
    file = '31enr_16_in.e'
  []
  [shift_z]
    type = TransformGenerator
    input = 'core'
    transform = 'TRANSLATE'
    vector_value = '0 0 100'
  []
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
[]

[RayKernels/collision]
  type = CollisionKernel
  temperature = temperature

  # mesh block ids
  blocks = "10000 10001 10004 10006 10008 10009 10010 10013 10014 11006"
  # openmc material id
  materials = "1 2 5 7 9 10 11 14 15 7"
  z_coord = 100
[]

[RayBCs]
  [reflect]
    type = ReflectRayBC
    boundary = '1 2 3 4'
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
  []
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
  csv = true
[]

# ==============================================================================
# TALLIES
# ==============================================================================

# [UserObjects]
#   inactive = 'tally univtally'
#
#   [tally]
#     type = OpenMCTally
#     particle_type = 'neutron'
#     estimator = 'COLLISION'
#     scores = 'flux scatter (n,fission) 16'
#     filters = 'energy particle'
#     energy_bins = '1e-5 1e3 2e7'
#     execute_on = 'initial'
#   []
#
#   [univtally]
#     type = OpenMCTally
#     particle_type = 'neutron'
#     estimator = 'COLLISION'
#     scores = 'kappa-fission'
#     filters = 'universe'
#     execute_on = 'initial'
#   []
#
#   [celltally]
#     type = OpenMCTally
#     id = 1
#     particle_type = 'neutron'
#     estimator = 'COLLISION'
#     scores = 'kappa-fission'
#     filters = 'cell'
#     execute_on = 'initial'
#   []
# []
#
# [AuxKernels]
#   [cell_val]
#     type = OpenMCTallyAux
#     granularity = 'cell'
#     score = 'kappa-fission'
#     tally_id = 1
#     execute_on = TIMESTEP_END
#     variable = power
#   []
# []
#
# [AuxVariables]
#   [power]
#     order = CONSTANT
#     family = MONOMIAL
#   []
# []
#
# Plot fission rates on a coarse pin-cell mesh
# [VectorPostprocessors]
#   [pin_powers]
#     type = NearestPointIntegralVariablePostprocessor
#     variable = 'power'
#     block = '10010'
#     points_file = pin_file
#   []
# []
#
# [MultiApps]
#   [pin_mesh]
#     type = TransientMultiApp
#     input_files = 'output_pin.i'
#   []
# []
#
# [Transfers]
#   [power_uo]
#     type = MultiAppUserObjectTransfer
#     multi_app = pin_mesh
#     direction = 'TO_MULTIAPP'
#     user_object = pin_powers
#     variable = pin_power
#   []
# []

# ==============================================================================
# SIMULATION PERFORMANCE STUDY
# ==============================================================================

# To look at domain decomposition
[AuxVariables/domains]
[]

[AuxKernels]
  [domains]
    type = ProcessorIDAux
    variable = domains
  []
[]

# Performance metrics
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

  # Particles per second to compare to openmc
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
  [particles_per_s]
    type = ParsedPostprocessor
    pp_names = 'total_num_rays total_time'
    function = 'total_num_rays / total_time'
  []
[]

[VectorPostprocessors]
  [per_proc_ray_tracing]
    type = PerProcessorRayTracingResultsVectorPostprocessor
    execute_on = TIMESTEP_END
    study = study
  []
[]
