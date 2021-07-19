[Tests]
  design = 'OpenMCTally.md'
  issues = '#2'
  [neutron_filter]
    type = Exodiff
    input = openmc_tally.i
    exodiff = neutron_filter.e
    cli_args = "Outputs/file_base=neutron_filter UserObjects/active='study neutron_filter'"
    requirement = "The system shall be able to specify a tally acting on neutrons only."
  []
  [energy_filter]
    ...
  []
  [collision_estimator]
    ...
  []
  [analog_estimator]
    ...
  []
  [fission_rate]
    ...
  []
