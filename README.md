MaCaw
=====

MaCaw is a MOOSE-based Monte Carlo neutral particle transport code. It is designed to
leverage the MOOSE ray tracing module for parallel scalability and OpenMC physics routines
for reduced development cost and flexibility.

To build MaCaw:
# Build OpenMC
git submodule init openmc
git submodule update
cd openmc
mkdir build
cd build
cmake ..
make -j 56
cd ../..

# Build MaCaw
make -j 56
