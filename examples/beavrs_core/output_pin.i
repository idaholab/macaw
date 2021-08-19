[Mesh]
  [gfm]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0.63
    xmax = 214.6
    ymin = -214.6
    ymax = 0.63
    nx = 170
    ny = 170
  []
[]

[AuxVariables]
  [pin_power]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Problem]
  solve = false
  skip_nl_system_check = true
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
[]
