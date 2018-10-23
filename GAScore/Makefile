################################################################################
# Include
################################################################################

################################################################################
# Prologue
################################################################################

MAKEFLAGS += --warn-undefined-variables
SHELL := bash
.SHELLFLAGS := -eu -o pipefail -c
.DEFAULT_GOAL := test
.DELETE_ON_ERROR:
.SUFFIXES:

################################################################################
# Variables
################################################################################

ifndef SHOAL_PATH
$(error SHOAL_PATH not set in env -- must be set to the absolute path of \
of the repository root. Did you source init.sh?)
endif

ifndef SONAR_PATH
$(error SONAR_PATH not set in env -- sonar is required for testing)
endif

ifndef SHOAL_VIVADO_HLS
$(error SHOAL_VIVADO_HLS not set in env -- must be set to the absolute path of \
of the Vivado HLS include/ directory. Did you source init.sh?)
endif

local_dir = $(SHOAL_PATH)/GAScore
obj_dir = $(local_dir)/build
bin_dir = $(local_dir)/build/bin
src_dir = $(local_dir)/src
test_dir = $(local_dir)/testbench
test_build_dir = $(test_dir)/build
include_dir = $(local_dir)/include
hls_dir = $(local_dir)/vivado_hls
vivado_dir = $(local_dir)/vivado

share_dir = $(SHOAL_PATH)/share
share_obj_dir = $(share_dir)/build
share_bin_dir = $(share_dir)/build/bin
share_src_dir = $(share_dir)/src
share_include_dir = $(share_dir)/include
share_testbench_dir = $(share_dir)/testbench

modules := am_rx am_tx xpams hold_buffer

obj = $(shell find $(obj_dir) -name '*.o' -printf '%f\n' | \
sort -k 1nr | cut -f2-)
share_obj = $(shell find $(share_obj_dir) -name '*.o' -printf '%f\n' | \
sort -k 1nr | cut -f2-)
dep = $(obj:%.o=$(obj_dir)/%.d)
dep += $(share_obj:%.o=$(share_obj_dir)/%.d)

CC = g++
CFLAGS = -g -Wall -I$(include_dir) -I$(share_include_dir) \
	-I$(SHOAL_VIVADO_HLS) \
	-Wno-unknown-pragmas -Wno-comment -Wno-shift-count-overflow -MMD -MP

################################################################################
# Body
################################################################################

.PHONY: test hw sim clean yaml

#-------------------------------------------------------------------------------
# C++ Testbench Execution
#-------------------------------------------------------------------------------

test_modules=$(patsubst %, test_%, $(modules))
test: $(test_modules)

define make-test
test-$1: config_$1 $(bin_dir)/$1_tb
	@$(bin_dir)/$1_tb
endef
$(foreach module, $(modules),$(eval $(call make-test,$(module))))

#-------------------------------------------------------------------------------
# Sonar Testbench Generation
#-------------------------------------------------------------------------------

config_modules=$(patsubst %, config_%, $(modules))
config: $(config_modules)

define make-config
config-$1:
	@python $(SONAR_PATH)/src/sonar.py env SHOAL_PATH /GAScore/testbench/$1.yaml
endef
$(foreach module, $(modules),$(eval $(call make-config,$(module))))

#-------------------------------------------------------------------------------
# Vivado HLS HW Generation
#-------------------------------------------------------------------------------

hw_modules=$(patsubst %, hw_%, $(modules))
hw: $(hw_modules)

define make-hw
hw-$1:
	@$(hls_dir)/generate.sh $1 # create Vivado HLS project and export the design
	@$(vivado_dir)/generate.sh $1 # Symlink all generated files for Vivado
endef
$(foreach module, $(modules),$(eval $(call make-hw,$(module))))

#-------------------------------------------------------------------------------
# Vivado RTL Simulation
#-------------------------------------------------------------------------------

define make-sim
sim-$1:
	@vivado -source $(vivado_dir)/generate.tcl -notrace -tclargs --project $1
endef
$(foreach module, $(modules),$(eval $(call make-sim,$(module))))

define make-batchsim
batchsim-$1:
	@vivado -mode batch -source $(vivado_dir)/generate.tcl -notrace -tclargs \
		--project $1 --sim behav
endef
$(foreach module, $(modules),$(eval $(call make-batchsim,$(module))))

#-------------------------------------------------------------------------------
# C++ Executable Generation
#-------------------------------------------------------------------------------

define make-executable
$(bin_dir)/$1_tb: $(obj_dir)/$1_tb.o $(obj_dir)/$1.o $(obj_dir)/utilities.o
	$(CC) $(CFLAGS) -o $(bin_dir)/$1_tb $(obj_dir)/$1_tb.o \
	$(obj_dir)/$1.o $(obj_dir)/utilities.o
endef
$(foreach module, $(modules),$(eval $(call make-executable,$(module))))

#-------------------------------------------------------------------------------
# Object Files Generation
#-------------------------------------------------------------------------------

define make-testbench
$(obj_dir)/$1_tb.o: $(test_build_dir)/$1_tb.cpp
	$(CC) $(CFLAGS) -o $(obj_dir)/$1_tb.o -c $(test_build_dir)/$1_tb.cpp
endef
$(foreach module, $(modules),$(eval $(call make-testbench,$(module))))

$(obj_dir)/utilities.o: $(src_dir)/utilities.cpp
	$(CC) $(CFLAGS) -o $(obj_dir)/utilities.o -c $(src_dir)/utilities.cpp

define make-object
$(obj_dir)/$1.o: $(src_dir)/$1.cpp
	$(CC) $(CFLAGS) -o $(obj_dir)/$1.o -c $(src_dir)/$1.cpp
endef
$(foreach module, $(modules),$(eval $(call make-object,$(module))))

-include $(dep)

#-------------------------------------------------------------------------------
# Cleanup
#-------------------------------------------------------------------------------

clean: 
	@$(RM) $(obj_dir)/*.o $(obj_dir)/*.d $(bin_dir)/* \
	$(share_obj_dir)/*.o $(share_obj_dir)/*.d $(share_bin_dir)/* \
	$(test_build_dir)/*