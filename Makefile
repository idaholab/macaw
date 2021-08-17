###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Optional Environment variables
# MOOSE_DIR        - Root directory of the MOOSE project
#
# To build:
# - (debug) add -D_GLIBCXX_DEBUG to openmc Makefile
# - make clobber_openmc (to remove pugixml found)
# - make build_openmc
# - make
#
# TODO: -Fix pugixml not found
#       -Fix debug vectors when used in openmc
#       -Find how to link libopenmc without LD_LIBRARY_PATH (-Wl,-rpath?)
# ======================================================================================
MACAW_DIR       := $(abspath $(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))))
CONTRIB_DIR     := $(MACAW_DIR)
MOOSE_SUBMODULE ?= $(CONTRIB_DIR)/moose
OPENMC_DIR      ?= $(CONTRIB_DIR)/openmc
PETSC_DIR       ?= $(MOOSE_SUBMODULE)/petsc
PETSC_ARCH      ?= arch-moose
LIBMESH_DIR     ?= $(MOOSE_SUBMODULE)/libmesh/installed/
HYPRE_DIR       ?= $(PETSC_DIR)/$(PETSC_ARCH)
CONTRIB_INSTALL_DIR ?= $(MACAW_DIR)/install
HDF5_INCLUDE_DIR ?= $(HDF5_ROOT)/include
HDF5_LIBDIR ?= $(HDF5_ROOT)/lib
HDF5_INCLUDES := -I$(HDF5_INCLUDE_DIR) -I$(HDF5_ROOT)/include
# BUILD_TYPE will be passed to CMake via CMAKE_BUILD_TYPE
ifeq ($(METHOD),opt)
	BUILD_TYPE := Release
else
	BUILD_TYPE := Debug
endif

# This needs to be exported
OPENMC_BUILDDIR := $(MACAW_DIR)/build/openmc
OPENMC_INSTALL_DIR := $(CONTRIB_DIR)/openmc/build
OPENMC_INCLUDES := -I$(OPENMC_DIR)/include -I$(OPENMC_INSTALL_DIR)/include -I$(OPENMC_DIR)/vendor/fmt/include -I$(OPENMC_DIR)/vendor/pugixml/src -I$(OPENMC_DIR)/vendor/xtensor/include -I$(OPENMC_DIR)/vendor/xtl/include -I$(OPENMC_DIR)/vendor/gsl-lite/include -I$(OPENMC_DIR)/include
OPENMC_LIBDIR := $(OPENMC_INSTALL_DIR)/lib
OPENMC_LIB := $(OPENMC_LIBDIR)/libopenmc.so

# This is used in $(FRAMEWORK_DIR)/build.mk
ADDITIONAL_CPPFLAGS := $(HDF5_INCLUDES) $(OPENMC_INCLUDES)
# ======================================================================================
# PETSc
# ======================================================================================
# Use compiler info discovered by PETSC
# include $(PETSC_DIR)/$(PETSC_ARCH)/lib/petsc/conf/petscvariables
# ======================================================================================
# MOOSE core objects
# ======================================================================================
# Use the MOOSE submodule if it exists and MOOSE_DIR is not set
MOOSE_SUBMODULE    := $(CURDIR)/moose
ifneq ($(wildcard $(MOOSE_SUBMODULE)/framework/Makefile),)
  MOOSE_DIR        ?= $(MOOSE_SUBMODULE)
else
  MOOSE_DIR        ?= $(shell dirname `pwd`)/moose
endif

# framework
FRAMEWORK_DIR      := $(MOOSE_DIR)/framework
include $(FRAMEWORK_DIR)/build.mk
include $(FRAMEWORK_DIR)/moose.mk

################################## MODULES ####################################
# To use certain physics included with MOOSE, set variables below to
# yes as needed.  Or set ALL_MODULES to yes to turn on everything (overrides
# other set variables).

ALL_MODULES                 := no

CHEMICAL_REACTIONS          := no
CONTACT                     := no
EXTERNAL_PETSC_SOLVER       := no
FLUID_PROPERTIES            := no
FUNCTIONAL_EXPANSION_TOOLS  := no
GEOCHEMISTRY                := no
HEAT_CONDUCTION             := no
LEVEL_SET                   := no
MISC                        := no
NAVIER_STOKES               := no
PHASE_FIELD                 := no
POROUS_FLOW                 := no
RAY_TRACING                 := yes
RDG                         := no
RICHARDS                    := no
STOCHASTIC_TOOLS            := no
TENSOR_MECHANICS            := no
XFEM                        := no

include $(MOOSE_DIR)/modules/modules.mk
###############################################################################

# dep apps
APPLICATION_DIR    := $(CURDIR)
APPLICATION_NAME   := macaw
BUILD_EXEC         := yes
GEN_REVISION       := no
include            $(MACAW_DIR)/config/openmc.mk
# ======================================================================================
# Building app objects defined in app.mk
# ======================================================================================
# ADDITIONAL_LIBS are used for linking in app.mk
# CC_LINKER_SLFLAG is from petscvariables
ADDITIONAL_LIBS := \
	-L$(MACAW_DIR)/lib \
	-L$(OPENMC_LIBDIR) \
	-Wl,-rpath,$(OPENMC_LIBDIR) \
	-lopenmc \
	-lhdf5_hl
include            $(FRAMEWORK_DIR)/app.mk

# app_objects are defined in moose.mk and built according to the rules in build.mk
# We need to build these first so we get include dirs
$(app_objects): build_openmc
$(test_objects): build_openmc

###############################################################################
# Additional special case targets should be added here
