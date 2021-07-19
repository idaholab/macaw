$(OPENMC_BUILDDIR)/Makefile: $(OPENMC_DIR)/CMakeLists.txt
				mkdir -p $(OPENMC_BUILDDIR)
				mkdir -p $(LIBMESH_DIR)/include/contrib
				cd $(OPENMC_BUILDDIR) && \
				cmake -L \
				-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
				-Doptimize=ON \
				-DCMAKE_PREFIX_PATH=$(LIBMESH_DIR) \
				-DCMAKE_INSTALL_PREFIX=$(OPENMC_INSTALL_DIR) \
				-DCMAKE_INSTALL_LIBDIR=$(OPENMC_LIBDIR) \
				$(OPENMC_DIR)
# 				-Dlibmesh=ON \ not needed for MaCaw

$(OPENMC_BUILDDIR)/Makefile_dbg: $(OPENMC_DIR)/CMakeLists.txt
				mkdir -p $(OPENMC_BUILDDIR)
				mkdir -p $(LIBMESH_DIR)/include/contrib
				cd $(OPENMC_BUILDDIR) && \
				cmake -L \
				-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
				-Ddebug=ON \
				-DCMAKE_PREFIX_PATH=$(LIBMESH_DIR) \
				-DCMAKE_INSTALL_PREFIX=$(OPENMC_INSTALL_DIR) \
				-DCMAKE_INSTALL_LIBDIR=$(OPENMC_LIBDIR) \
				-E env CMAKE_CXX_FLAGS="-D_GLIBCXX_DEBUG" \
				$(OPENMC_DIR)
# 				-Dlibmesh=ON \ not needed for MaCaw

build_openmc: | $(OPENMC_BUILDDIR)/Makefile
				make ${MAKEFLAGS} VERBOSE=1 -C $(OPENMC_BUILDDIR) install

build_openmc_dbg: | $(OPENMC_BUILDDIR)/Makefile_dbg
				make ${MAKEFLAGS} VERBOSE=1 -C $(OPENMC_BUILDDIR) install

cleanall_openmc: | $(OPENMC_BUILDDIR)/Makefile
					make -C $(OPENMC_BUILDDIR) clean

clobber_openmc:
				rm -rf $(OPENMC_LIB) $(OPENMC_BUILDDIR) $(OPENMC_INSTALL_DIR)

cleanall: cleanall_openmc

clobberall: clobber_openmc

.PHONY: build_openmc cleanall_openmc clobber_openmc
