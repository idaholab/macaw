folder = '/home/guillaume/projects/mockingbird/problems/beavrs/generator_geom'

[Mesh]
  [fuel_31]
    type = FileMeshGenerator
    file = '${folder}/pin_cells/fuel_31enr_coarse.e'
  []
  [bp]
    type = FileMeshGenerator
    file = '${folder}/pin_cells/bp_coarse.e'
  []
  [gt]
    type = FileMeshGenerator
    file = '${folder}/pin_cells/guide_tube_coarse.e'
  []
  [it]
    type = FileMeshGenerator
    file = '${folder}/pin_cells/guide_tube_coarse.e'
  []
  [assembly]
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
    distribution = SERIAL
    bottom_boundary = 1
    right_boundary = 2
    top_boundary = 3
    left_boundary = 4
  []
[]

[Outputs]
  exodus = true
[]
