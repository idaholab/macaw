from math import log10

import matplotlib.pyplot as plt
import numpy as np
import openmc
import os

# Strong scaling study
# Increase the number of processors, keep the load (neutrons, domain size) the same

mode = 'openmp'
mode = 'mpi'

num_procs = [1, 2, 4, 8, 16, 28, 32, 40, 50, 56, 64, 80, 112]
timings = [[], []]

# Some parameters for the study
n_batches = 100
n_active_batches = 90
n_neutrons = int(1e2)

for num_proc in num_procs:

    width = np.sqrt(num_proc)
    neutrons = num_proc * n_neutrons

    ###############################################################################
    # Create materials for the problem

    inf_medium = openmc.Material(name='moderator')
    inf_medium.set_density('g/cc', 5.)
    inf_medium.add_nuclide('H1',  0.03)
    inf_medium.add_nuclide('O16', 0.015)
    inf_medium.add_nuclide('U235', 0.0001)
    inf_medium.add_nuclide('U238', 0.007)
    inf_medium.add_nuclide('Pu239', 0.00003)
    inf_medium.add_nuclide('Zr90', 0.002)

    # Collect the materials together and export to XML
    materials = openmc.Materials([inf_medium])
    materials.export_to_xml()

    ###############################################################################
    # Define problem geometry

    # Create a region represented as the inside of a rectangular prism
    pitch = 1.25984 * width
    box = openmc.rectangular_prism(pitch, pitch, boundary_type='reflective')

    # Create cells, mapping materials to regions
    water = openmc.Cell(fill=inf_medium, region=box)

    # Create a geometry and export to XML
    geometry = openmc.Geometry([water])
    geometry.export_to_xml()

    ###############################################################################
    # Define problem settings

    # Indicate how many particles to run
    settings = openmc.Settings()
    settings.batches = n_batches
    settings.inactive = n_batches - n_active_batches
    settings.particles = neutrons

    # Create an initial uniform spatial source distribution over fissionable zones
    lower_left = (-pitch/2, -pitch/2, -1)
    upper_right = (pitch/2, pitch/2, 1)
    uniform_dist = openmc.stats.Box(lower_left, upper_right, only_fissionable=True)
    settings.source = openmc.source.Source(space=uniform_dist)

    settings.export_to_xml()

    ###############################################################################
    # Define tallies

    # Create a mesh that will be used for tallying
    mesh = openmc.RegularMesh()
    mesh.dimension = (100, 100)
    mesh.lower_left = (-pitch/2, -pitch/2)
    mesh.upper_right = (pitch/2, pitch/2)

    # Create a mesh filter that can be used in a tally
    mesh_filter = openmc.MeshFilter(mesh)

    # Now use the mesh filter in a tally and indicate what scores are desired
    mesh_tally = openmc.Tally(name="Mesh tally")
    mesh_tally.filters = [mesh_filter]
    mesh_tally.scores = ['flux', 'fission', 'nu-fission']

    # Let's also create a tally to get the flux energy spectrum. We start by
    # creating an energy filter
    e_min, e_max = 1e-5, 20.0e6
    groups = 500
    energies = np.logspace(log10(e_min), log10(e_max), groups + 1)
    energy_filter = openmc.EnergyFilter(energies)

    # Instantiate a Tallies collection and export to XML
    tallies = openmc.Tallies([mesh_tally])
    tallies.export_to_xml()

    if mode == 'openmp':
        openmc.run("--threads", num_proc)
    elif mode == 'mpi':
        os.system("mpirun -n "+str(num_proc)+" openmc -s 1")
    else:
      print("Unrecognized run mode")

    # Get run time
    with openmc.StatePoint("statepoint.100.h5") as statepoint:
        timings[0].append(statepoint.runtime['active batches'])


# Plot results
plt.figure()
plt.loglog(num_procs, timings[0])
plt.title("OpenMC "+mode+"scaling")
plt.xlabel("Number of processors (-)")
plt.ylabel("Active batch time (s)")
plt.tight_layout()
plt.savefig("openmc_scaling_" + mode + "_time")

plt.figure()
plt.loglog(num_procs, np.array(num_procs) * n_active_batches * n_neutrons / timings[0])
plt.title("OpenMC scaling: "+mode)
plt.xlabel("Number of processors (-)")
plt.ylabel("Active neutrons per s (-)")
plt.tight_layout()
plt.savefig("openmc_scaling_" + mode + "_neutrons")
