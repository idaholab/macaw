folder = '/home/guillaume/projects/mockingbird/problems/beavrs/generator_geom'

[Mesh]
  # active = 'fuel_16 fuel_24 fuel_31 bp gt it water_assembly outer_water'
  inactive = 'core'
  # inactive = 'small_core delete_left delete_top'
  # active = 'fuel_31 fuel_16 bp gt it sleeve 16enr_no_instr_pins 16enr_no_instr'

  [fuel_31]
    type = FileMeshGenerator
    file = '${folder}/pin_cells/new_fuel_31enr_8.e'
  []
  [fuel_24]
    type = RenameBlockGenerator
    input = fuel_31
    old_block_id = '10010'
    new_block_id = '10009'
  []
  [fuel_16]
    type = RenameBlockGenerator
    input = fuel_31
    old_block_id = '10010'
    new_block_id = '10008'
  []
  [bp]
    type = FileMeshGenerator
    file = '${folder}/pin_cells/new_bp_8.e'
  []
  [gt]
    type = FileMeshGenerator
    file = '${folder}/pin_cells/new_gt_8.e'
  []
  [it]
    type = FileMeshGenerator
    file = '${folder}/pin_cells/new_gt_8.e'
  []
  [sleeve]
    type = FileMeshGenerator
    file = '${folder}/assemblies/sleeve_gap_8.e'
  []

#---------------------assemblies-----------------------------
  [16enr_no_instr_pins]
    type = PatternedMeshGenerator
    inputs = 'fuel_16 bp gt it'
    pattern = '0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 2 0 0 2 0 0 2 0 0 0 0 0 ;
               0 0 0 2 0 0 0 0 0 0 0 0 0 2 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 2 0 0 2 0 0 2 0 0 2 0 0 2 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 2 0 0 2 0 0 3 0 0 2 0 0 2 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 2 0 0 2 0 0 2 0 0 2 0 0 2 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 2 0 0 0 0 0 0 0 0 0 2 0 0 0 ;
               0 0 0 0 0 2 0 0 2 0 0 2 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0'
    y_width = 1.25984
    x_width = 1.25984
    bottom_boundary = 1
    right_boundary = 2
    top_boundary = 3
    left_boundary = 4
  []
  [16enr_no_instr_rb]
    type = RenameBoundaryGenerator
    input = 16enr_no_instr_pins
    old_boundary_id = '1  2  3  4'
    new_boundary_id = '50 50 50 50'
  []
  [16enr_no_instr]
    type = StitchedMeshGenerator
    inputs = '16enr_no_instr_rb sleeve'
    stitch_boundaries_pairs = '50 51'
  []

  [31enr_no_instr_pins]
    type = PatternedMeshGenerator
    inputs = 'fuel_31 bp gt it'
    pattern = '0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 2 0 0 2 0 0 2 0 0 0 0 0 ;
               0 0 0 2 0 0 0 0 0 0 0 0 0 2 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 2 0 0 2 0 0 2 0 0 2 0 0 2 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 2 0 0 2 0 0 3 0 0 2 0 0 2 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 2 0 0 2 0 0 2 0 0 2 0 0 2 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 2 0 0 0 0 0 0 0 0 0 2 0 0 0 ;
               0 0 0 0 0 2 0 0 2 0 0 2 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0'
    y_width = 1.25984
    x_width = 1.25984
    bottom_boundary = 1
    right_boundary = 2
    top_boundary = 3
    left_boundary = 4
  []
  [31enr_no_instr_rb]
    type = RenameBoundaryGenerator
    input = 31enr_no_instr_pins
    old_boundary_id = '1  2  3  4'
    new_boundary_id = '50 50 50 50'
  []
  [31enr_no_instr]
    type = StitchedMeshGenerator
    inputs = '31enr_no_instr_rb sleeve'
    stitch_boundaries_pairs = '50 51'
  []

  [31enr_6n_pins]
    type = PatternedMeshGenerator
    inputs = 'fuel_31 bp gt it'
    pattern = '0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 1 0 0 2 0 0 1 0 0 0 0 0 ;
               0 0 0 1 0 0 0 0 0 0 0 0 0 1 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 1 0 0 2 0 0 2 0 0 2 0 0 1 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 2 0 0 2 0 0 3 0 0 2 0 0 2 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 2 0 0 2 0 0 2 0 0 2 0 0 2 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 2 0 0 0 0 0 0 0 0 0 2 0 0 0 ;
               0 0 0 0 0 2 0 0 2 0 0 2 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0'
    y_width = 1.25984
    x_width = 1.25984
    bottom_boundary = 1
    right_boundary = 2
    top_boundary = 3
    left_boundary = 4
  []
  [31enr_6n_rb]
    type = RenameBoundaryGenerator
    input = 31enr_6n_pins
    old_boundary_id = '1  2  3  4'
    new_boundary_id = '50 50 50 50'
  []
  [31enr_6n]
    type = StitchedMeshGenerator
    inputs = '31enr_6n_rb sleeve'
    stitch_boundaries_pairs = '50 51'
  []

  # FIXME There is no reason why this would not work, and yet...
  # [31enr_6w_rotate]
  #   type = TransformGenerator
  #   input = 31enr_6n
  #   transform = ROTATE
  #   vector_value = '90 0 0'
  # []
  # [31enr_6w_id]
  #   type = RenameBoundaryGenerator
  #   input = 31enr_6w_rotate
  #   old_boundary_id = '1 4 3 2'
  #   new_boundary_id = '2 1 4 3'
  # []
  # [31enr_6w]
  #   type = TransformGenerator
  #   input = 31enr_6w_id
  #   transform = TRANSLATE
  #   vector_value = '0 -20.15744 0'
  # []

  [31enr_6w_pins]
    type = PatternedMeshGenerator
    inputs = 'fuel_31 bp gt it'
    pattern = '0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 1 0 0 2 0 0 2 0 0 0 0 0 ;
               0 0 0 1 0 0 0 0 0 0 0 0 0 2 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 1 0 0 2 0 0 2 0 0 2 0 0 2 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 2 0 0 2 0 0 3 0 0 2 0 0 2 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 1 0 0 2 0 0 2 0 0 2 0 0 2 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 1 0 0 0 0 0 0 0 0 0 2 0 0 0 ;
               0 0 0 0 0 1 0 0 2 0 0 2 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0'
    y_width = 1.25984
    x_width = 1.25984
    bottom_boundary = 1
    right_boundary = 2
    top_boundary = 3
    left_boundary = 4
  []
  [31enr_6w_rb]
    type = RenameBoundaryGenerator
    input = 31enr_6w_pins
    old_boundary_id = '1  2  3  4'
    new_boundary_id = '50 50 50 50'
  []
  [31enr_6w]
    type = StitchedMeshGenerator
    inputs = '31enr_6w_rb sleeve'
    stitch_boundaries_pairs = '50 51'
  []

  [31enr_6s_rotate]
    type = TransformGenerator
    input = 31enr_6n
    transform = ROTATE
    vector_value = '180 0 0'
  []
  [31enr_6s_id]
    type = RenameBoundaryGenerator
    input = 31enr_6s_rotate
    old_boundary_id = '1 4 3 2'
    new_boundary_id = '3 2 1 4'
  []
  [31enr_6s]
    type = TransformGenerator
    input = 31enr_6s_id
    transform = TRANSLATE
    vector_value = '20.15744 -20.15744 0'
  []

  [31enr_6e_rotate]
    type = TransformGenerator
    input = 31enr_6n
    transform = ROTATE
    vector_value = '270 0 0'
  []
  [31enr_6e_id]
    type = RenameBoundaryGenerator
    input = 31enr_6e_rotate
    old_boundary_id = '1 4 3 2'
    new_boundary_id = '4 3 2 1'
  []
  [31enr_6e]
    type = TransformGenerator
    input = 31enr_6e_id
    transform = TRANSLATE
    vector_value = '20.15744 0 0'
  []

  [31enr_20_pins]
    type = PatternedMeshGenerator
    inputs = 'fuel_31 bp gt it'
    pattern = '0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 1 0 0 1 0 0 1 0 0 0 0 0 ;
               0 0 0 1 0 0 0 0 0 0 0 0 0 1 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 1 0 0 1 0 0 2 0 0 1 0 0 1 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 1 0 0 2 0 0 3 0 0 2 0 0 1 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 1 0 0 1 0 0 2 0 0 1 0 0 1 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 1 0 0 0 0 0 0 0 0 0 1 0 0 0 ;
               0 0 0 0 0 1 0 0 1 0 0 1 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0'
    y_width = 1.25984
    x_width = 1.25984
    bottom_boundary = 1
    right_boundary = 2
    top_boundary = 3
    left_boundary = 4
  []
  [31enr_20_rb]
    type = RenameBoundaryGenerator
    input = 31enr_20_pins
    old_boundary_id = '1  2  3  4'
    new_boundary_id = '50 50 50 50'
  []
  [31enr_20]
    type = StitchedMeshGenerator
    inputs = '31enr_20_rb sleeve'
    stitch_boundaries_pairs = '50 51'
  []

  [31enr_16_pins]
    type = PatternedMeshGenerator
    inputs = 'fuel_31 bp gt it'
    pattern = '0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 1 0 0 1 0 0 1 0 0 0 0 0 ;
               0 0 0 1 0 0 0 0 0 0 0 0 0 1 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 1 0 0 2 0 0 2 0 0 2 0 0 1 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 1 0 0 2 0 0 3 0 0 2 0 0 1 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 1 0 0 2 0 0 2 0 0 2 0 0 1 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 1 0 0 0 0 0 0 0 0 0 1 0 0 0 ;
               0 0 0 0 0 1 0 0 1 0 0 1 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0'
    y_width = 1.25984
    x_width = 1.25984
    bottom_boundary = 1
    right_boundary = 2
    top_boundary = 3
    left_boundary = 4
  []
  [31enr_16_rb]
    type = RenameBoundaryGenerator
    input = 31enr_16_pins
    old_boundary_id = '1  2  3  4'
    new_boundary_id = '50 50 50 50'
  []
  [31enr_16]
    type = StitchedMeshGenerator
    inputs = '31enr_16_rb sleeve'
    stitch_boundaries_pairs = '50 51'
  []

  [31enr_15nw_pins]
    type = PatternedMeshGenerator
    inputs = 'fuel_31 bp gt it'
    pattern = '0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 1 0 0 1 0 0 1 0 0 0 0 0 ;
               0 0 0 1 0 0 0 0 0 0 0 0 0 2 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 1 0 0 1 0 0 1 0 0 1 0 0 2 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 1 0 0 1 0 0 3 0 0 1 0 0 2 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 1 0 0 1 0 0 1 0 0 1 0 0 2 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 2 0 0 0 0 0 0 0 0 0 2 0 0 0 ;
               0 0 0 0 0 2 0 0 2 0 0 2 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0'
    y_width = 1.25984
    x_width = 1.25984
    bottom_boundary = 1
    right_boundary = 2
    top_boundary = 3
    left_boundary = 4
  []
  [31enr_15nw_rb]
    type = RenameBoundaryGenerator
    input = 31enr_15nw_pins
    old_boundary_id = '1  2  3  4'
    new_boundary_id = '50 50 50 50'
  []
  [31enr_15nw]
    type = StitchedMeshGenerator
    inputs = '31enr_15nw_rb sleeve'
    stitch_boundaries_pairs = '50 51'
  []

  # FIXME
  [31enr_15sw_rotate]
    type = TransformGenerator
    input = 31enr_15nw
    transform = ROTATE
    vector_value = '90 0 0'
  []
  [31enr_15sw_id]
    type = RenameBoundaryGenerator
    input = 31enr_15sw_rotate
    old_boundary_id = '1 4 3 2'
    new_boundary_id = '2 1 4 3'
  []
  [31enr_15sw]
    type = TransformGenerator
    input = 31enr_15sw_id
    transform = TRANSLATE
    vector_value = '0 -20.15744 0'
  []

  [31enr_15se_rotate]
    type = TransformGenerator
    input = 31enr_15nw
    transform = ROTATE
    vector_value = '180 0 0'
  []
  [31enr_15se_id]
    type = RenameBoundaryGenerator
    input = 31enr_15se_rotate
    old_boundary_id = '1 4 3 2'
    new_boundary_id = '3 2 1 4'
  []
  [31enr_15se]
    type = TransformGenerator
    input = 31enr_15se_id
    transform = TRANSLATE
    vector_value = '20.15744 -20.15744 0'
  []
  [31enr_15ne_rotate]
    type = TransformGenerator
    input = 31enr_15nw
    transform = ROTATE
    vector_value = '270 0 0'
  []
  [31enr_15ne_id]
    type = RenameBoundaryGenerator
    input = 31enr_15ne_rotate
    old_boundary_id = '1 4 3 2'
    new_boundary_id = '4 3 2 1'
  []
  [31enr_15ne]
    type = TransformGenerator
    input = 31enr_15ne_id
    transform = TRANSLATE
    vector_value = '20.15744 0 0'
  []
  [24enr_no_instr_pins]
    type = PatternedMeshGenerator
    inputs = 'fuel_24 bp gt it'
    pattern = '0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 2 0 0 2 0 0 2 0 0 0 0 0 ;
               0 0 0 2 0 0 0 0 0 0 0 0 0 2 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 2 0 0 2 0 0 2 0 0 2 0 0 2 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 2 0 0 2 0 0 3 0 0 2 0 0 2 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 2 0 0 2 0 0 2 0 0 2 0 0 2 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 2 0 0 0 0 0 0 0 0 0 2 0 0 0 ;
               0 0 0 0 0 2 0 0 2 0 0 2 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0'
    y_width = 1.25984
    x_width = 1.25984
    bottom_boundary = 1
    right_boundary = 2
    top_boundary = 3
    left_boundary = 4
  []
  [24enr_no_instr_rb]
    type = RenameBoundaryGenerator
    input = 24enr_no_instr_pins
    old_boundary_id = '1  2  3  4'
    new_boundary_id = '50 50 50 50'
  []
  [24enr_no_instr]
    type = StitchedMeshGenerator
    inputs = '24enr_no_instr_rb sleeve'
    stitch_boundaries_pairs = '50 51'
  []


  [24enr_12_pins]
    type = PatternedMeshGenerator
    inputs = 'fuel_24 bp gt it'
    pattern = '0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 1 0 0 2 0 0 1 0 0 0 0 0 ;
               0 0 0 1 0 0 0 0 0 0 0 0 0 1 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 1 0 0 2 0 0 2 0 0 2 0 0 1 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 2 0 0 2 0 0 3 0 0 2 0 0 2 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 1 0 0 2 0 0 2 0 0 2 0 0 1 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 1 0 0 0 0 0 0 0 0 0 1 0 0 0 ;
               0 0 0 0 0 1 0 0 2 0 0 1 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0'
    y_width = 1.25984
    x_width = 1.25984
    bottom_boundary = 1
    right_boundary = 2
    top_boundary = 3
    left_boundary = 4
  []
  [24enr_12_rb]
    type = RenameBoundaryGenerator
    input = 24enr_12_pins
    old_boundary_id = '1  2  3  4'
    new_boundary_id = '50 50 50 50'
  []
  [24enr_12]
    type = StitchedMeshGenerator
    inputs = '24enr_12_rb sleeve'
    stitch_boundaries_pairs = '50 51'
  []


  [24enr_16_pins]
    type = PatternedMeshGenerator
    inputs = 'fuel_24 bp gt it'
    pattern = '0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 1 0 0 1 0 0 1 0 0 0 0 0 ;
               0 0 0 1 0 0 0 0 0 0 0 0 0 1 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 1 0 0 2 0 0 2 0 0 2 0 0 1 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 1 0 0 2 0 0 3 0 0 2 0 0 1 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 1 0 0 2 0 0 2 0 0 2 0 0 1 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 1 0 0 0 0 0 0 0 0 0 1 0 0 0 ;
               0 0 0 0 0 1 0 0 1 0 0 1 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0'
    y_width = 1.25984
    x_width = 1.25984
    bottom_boundary = 1
    right_boundary = 2
    top_boundary = 3
    left_boundary = 4
  []
  [24enr_16_rb]
    type = RenameBoundaryGenerator
    input = 24enr_16_pins
    old_boundary_id = '1  2  3  4'
    new_boundary_id = '50 50 50 50'
  []
  [24enr_16]
    type = StitchedMeshGenerator
    inputs = '24enr_16_rb sleeve'
    stitch_boundaries_pairs = '50 51'
  []

  [water_assembly]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 136
    xmin = -0.673100
    xmax = 20.830540
    ny = 136
    ymin = -20.830540
    ymax = 0.673100
  []
  [water_assembly_ids]
    type = RenameBoundaryGenerator
    input = water_assembly
    old_boundary_id = '0 1 2 3'
    new_boundary_id = '1 2 3 4'
  []
  [outer_water]
    type = SubdomainIDGenerator
    input = water_assembly_ids
    subdomain_id = 10016
  []

  [water_baffle_nw_not_moved]
    type = FileMeshGenerator
    file = '${folder}/assemblies/water_baffle_2sides_8.e'
  []
  [water_baffle_nw]
    type = TransformGenerator
    input = water_baffle_nw_not_moved
    transform = TRANSLATE
    vector_value = '10.07872 -10.07872 0'
  []

  #FIXME
  [water_baffle_sw_rotate]
    type = TransformGenerator
    input = water_baffle_nw_not_moved
    transform = ROTATE
    vector_value = '90 0 0'
  []
  [water_baffle_sw_move]
    type = TransformGenerator
    input = water_baffle_sw_rotate
    transform = TRANSLATE
    vector_value = '10.07872 -10.07872 0'
  []
  [water_baffle_sw]
    type = RenameBoundaryGenerator
    input = water_baffle_sw_move
    old_boundary_id = '1 4 3 2'
    new_boundary_id = '2 1 4 3'
  []

  [water_baffle_se_rotate]
    type = TransformGenerator
    input = water_baffle_nw_not_moved
    transform = ROTATE
    vector_value = '180 0 0'
  []
  [water_baffle_se_move]
    type = TransformGenerator
    input = water_baffle_se_rotate
    transform = TRANSLATE
    vector_value = '10.07872 -10.07872 0'
  []
  [water_baffle_se]
    type = RenameBoundaryGenerator
    input = water_baffle_se_move
    old_boundary_id = '1 4 3 2'
    new_boundary_id = '3 2 1 4'
  []

  [water_baffle_ne_rotate]
    type = TransformGenerator
    input = water_baffle_nw_not_moved
    transform = ROTATE
    vector_value = '270 0 0'
  []
  [water_baffle_ne_move]
    type = TransformGenerator
    input = water_baffle_ne_rotate
    transform = TRANSLATE
    vector_value = '10.07872 -10.07872 0'
  []
  [water_baffle_ne]
    type = RenameBoundaryGenerator
    input = water_baffle_ne_move
    old_boundary_id = '1 4 3 2'
    new_boundary_id = '4 3 2 1'
  []


  [water_baffle_w_not_moved]
    type = FileMeshGenerator
    file = '${folder}/assemblies/water_baffle_1side_8.e'
  []
  [water_baffle_w]
    type = TransformGenerator
    input = water_baffle_w_not_moved
    transform = TRANSLATE
    vector_value = '10.07872 -10.07872 0'
  []

  #FIXME
  [water_baffle_s_rotate]
    type = TransformGenerator
    input = water_baffle_w_not_moved
    transform = ROTATE
    vector_value = '90 0 0'
  []
  [water_baffle_s_move]
    type = TransformGenerator
    input = water_baffle_s_rotate
    transform = TRANSLATE
    vector_value = '10.07872 -10.07872 0'
  []
  [water_baffle_s]
    type = RenameBoundaryGenerator
    input = water_baffle_s_move
    old_boundary_id = '1 4 3 2'
    new_boundary_id = '2 1 4 3'
  []

  [water_baffle_e_rotate]
    type = TransformGenerator
    input = water_baffle_w_not_moved
    transform = ROTATE
    vector_value = '180 0 0'
  []
  [water_baffle_e_move]
    type = TransformGenerator
    input = water_baffle_e_rotate
    transform = TRANSLATE
    vector_value = '10.07872 -10.07872 0'
  []
  [water_baffle_e]
    type = RenameBoundaryGenerator
    input = water_baffle_e_move
    old_boundary_id = '1 4 3 2'
    new_boundary_id = '3 2 1 4'
  []

  [water_baffle_n_rotate]
    type = TransformGenerator
    input = water_baffle_w_not_moved
    transform = ROTATE
    vector_value = '270 0 0'
  []
  [water_baffle_n_move]
    type = TransformGenerator
    input = water_baffle_n_rotate
    transform = TRANSLATE
    vector_value = '10.07872 -10.07872 0'
  []
  [water_baffle_n]
    type = RenameBoundaryGenerator
    input = water_baffle_n_move
    old_boundary_id = '1 4 3 2'
    new_boundary_id = '4 3 2 1'
  []


  [water_baffle_corner_nw_not_moved]
    type = FileMeshGenerator
    file = '${folder}/assemblies/water_baffle_corner_8.e'
  []
  [water_baffle_corner_nw]
    type = TransformGenerator
    input = water_baffle_corner_nw_not_moved
    transform = TRANSLATE
    vector_value = '10.07872 -10.07872 0'
  []

  [water_baffle_corner_sw_rotate]
    type = TransformGenerator
    input = water_baffle_corner_nw_not_moved
    transform = ROTATE
    vector_value = '90 0 0'
  []
  [water_baffle_corner_sw_move]
    type = TransformGenerator
    input = water_baffle_corner_sw_rotate
    transform = TRANSLATE
    vector_value = '10.07872 -10.07872 0'
  []
  [water_baffle_corner_sw]
    type = RenameBoundaryGenerator
    input = water_baffle_corner_sw_move
    old_boundary_id = '1 4 3 2'
    new_boundary_id = '2 1 4 3'
  []

  [water_baffle_corner_se_rotate]
    type = TransformGenerator
    input = water_baffle_corner_nw_not_moved
    transform = ROTATE
    vector_value = '180 0 0'
  []
  [water_baffle_corner_se_move]
    type = TransformGenerator
    input = water_baffle_corner_se_rotate
    transform = TRANSLATE
    vector_value = '10.07872 -10.07872 0'
  []
  [water_baffle_corner_se]
    type = RenameBoundaryGenerator
    input = water_baffle_corner_se_move
    old_boundary_id = '1 4 3 2'
    new_boundary_id = '3 2 1 4'
  []

  [water_baffle_corner_ne_rotate]
    type = TransformGenerator
    input = water_baffle_corner_nw_not_moved
    transform = ROTATE
    vector_value = '270 0 0'
  []
  [water_baffle_corner_ne_move]
    type = TransformGenerator
    input = water_baffle_corner_ne_rotate
    transform = TRANSLATE
    vector_value = '10.07872 -10.07872 0'
  []
  [water_baffle_corner_ne]
    type = RenameBoundaryGenerator
    input = water_baffle_corner_ne_move
    old_boundary_id = '1 4 3 2'
    new_boundary_id = '4 3 2 1'
  []
#--------------------------------------------------------
  [core]
    type = PatternedMeshGenerator
    #               0           1        2         3        4          5           6
    inputs = '16enr_no_instr 24enr_16 24enr_12 31enr_6w 31enr_20 31enr_no_instr 31enr_16

24enr_no_instr 31enr_15nw 31enr_6n outer_water 31enr_6e 31enr_6s 31enr_15sw 31enr_15se

31enr_15ne water_baffle_nw water_baffle_sw water_baffle_se water_baffle_ne water_baffle_w

water_baffle_s water_baffle_e water_baffle_n water_baffle_corner_nw water_baffle_corner_sw

water_baffle_corner_se water_baffle_corner_ne'
#           26                  27
    pattern = '10 10 10 10 26 21 21 21 21 21 21 21 25 10 10 10 10;
               10 10 26 21 18  5 12  5 12  5 12  5 17 21 25 10 10;
               10 26 18  5  5  6  0  4  0  4  0  6  5  5 17 25 10;
               10 22  5 14  1  0  1  0  1  0  1  0  1 13  5 20 10;
               26 18  5  1  7  1  0  2  0  2  0  1  7  1  5 17 25;
               22  5  6  0  1  0  2  0  2  0  2  0  1  0  6  5 20;
               22 11  0  1  0  2  0  2  0  2  0  2  0  1  0  3 20;
               22  5  4  0  2  0  2  0  1  0  2  0  2  0  4  5 20;
               22 11  0  1  0  2  0  1  0  1  0  2  0  1  0  3 20;
               22  5  4  0  2  0  2  0  1  0  2  0  2  0  4  5 20;
               22 11  0  1  0  2  0  2  0  2  0  2  0  1  0  3 20;
               22  5  6  0  1  0  2  0  2  0  2  0  1  0  6  5 20;
               27 19  5  1  7  1  0  2  0  2  0  1  7  1  5 16 24;
               10 22  5 15  1  0  1  0  1  0  1  0  1  8  5 20 10;
               10 27 19  5  5  6  0  4  0  4  0  6  5  5 16 24 10;
               10 10 27 23 19  5  9  5  9  5  9  5 16 23 24 10 10;
               10 10 10 10 27 23 23 23 23 23 23 23 24 10 10 10 10'
    bottom_boundary = 1
    right_boundary = 2
    top_boundary = 3
    left_boundary = 4
  []

  [small_core]
    type = PatternedMeshGenerator
    #               0           1        2         3        4          5           6
    inputs = '16enr_no_instr 24enr_16 24enr_12 31enr_6w 31enr_20 31enr_no_instr 31enr_16

24enr_no_instr 31enr_15nw 31enr_6n outer_water 31enr_6e 31enr_6s 31enr_15sw 31enr_15se

31enr_15ne water_baffle_nw water_baffle_sw water_baffle_se water_baffle_ne water_baffle_w

water_baffle_s water_baffle_e water_baffle_n water_baffle_corner_nw water_baffle_corner_sw

water_baffle_corner_se water_baffle_corner_ne'
#           26                  27
    pattern = ' 0  2  0  2  0  2  0  1  0  3;
                2  0  1  0  2  0  2  0  4  5;
                0  1  0  1  0  2  0  1  0  3;
                2  0  1  0  2  0  2  0  4  5;
                0  2  0  2  0  2  0  1  0  3;
                2  0  2  0  2  0  1  0  6  5;
                0  2  0  2  0  1  7  1  5;
                1  0  1  0  1  0  1  8  5;
                0  4  0  4  0  6  5  5;
                9  5  9  5  9  5;'

    # pattern = '2 0;
    #            0 2;'

    # pattern = '2'

    bottom_boundary = 1
    right_boundary = 2
    top_boundary = 3
    left_boundary = 4
  []


  [center]
    # Center the mesh around 0,0
    type = TransformGenerator
    input = small_core
    transform = TRANSLATE
    vector_value = '0 0 0' #'-182.10784 182.10784 0'
  []

  # Delete the boundary for the quarter core boundary conditions
  [delete_left]
    type = PlaneDeletionGenerator
    input = center
    point = '0 0 0'
    normal = '-1 0 0'
    new_boundary = 4
  []
  [delete_top]
    type = PlaneDeletionGenerator
    input = delete_left
    point = '0 0 0'
    normal = '0 1 0'
    new_boundary = 3
  []
[]

[Problem]
  solve = false
[]

# To check the geometry, one can run diffusion on it
[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [bottom]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  []

  [top]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  nl_max_its = 0
[]

[Outputs]
  exodus = true
[]
