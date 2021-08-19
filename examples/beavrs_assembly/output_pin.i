[Mesh]
  [gfm]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 21.4
    ymin = -21.4
    ymax = 0
    nx = 17
    ny = 17
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
