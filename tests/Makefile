################################################################################
# Include
################################################################################

include $(SHOAL_PATH)/include.mk

################################################################################
# Prologue
################################################################################

MAKEFLAGS += --warn-undefined-variables
SHELL := bash
.SHELLFLAGS := -eu -o pipefail -c
# .DEFAULT_GOAL := 
.DELETE_ON_ERROR:
.SUFFIXES:

################################################################################
# Variables
################################################################################

test_dir := $(SHOAL_PATH)/tests

c_modules := node_2_kern_2_1

################################################################################
# Body
################################################################################

# Overwrites .dat file in sim folders if the file already exists there
define make-config
config-$1: guard-SONAR_PATH
	@python $(test_dir)/$1.py
	@$(eval $1_sim_path = $(test_dir)/projects/$(SHOAL_VIVADO_VERSION)/$(SHOAL_PART)/$1/$1.sim/sim_1/behav)
	@if [[ -f $($1_sim_path)/$1_sv.dat ]]; then cp $(test_build_dir)/$1/$1_sv.dat $($1_sim_path); fi
	@$(eval $1_sim_path = $(test_dir)/projects/$(SHOAL_VIVADO_VERSION)/$(SHOAL_PART)/$1/$1.sim/sim_1/synth/func)
	@if [[ -f $($1_sim_path)/$1_sv.dat ]]; then cp $(test_build_dir)/$1/$1_sv.dat $($1_sim_path); fi
endef
$(foreach module, $(c_modules),$(eval $(call make-config,$(module),all)))

define make-sim
sim-$1: guard-VIV_MODE guard-VIV_SIM guard-VIV_CREATE guard-VIV_SYNTH guard-VIV_IMPL guard-VIV_BIT guard-VIV_EXPORT
	@if [[ $(VIV_CREATE) == 1 ]]; then $(test_dir)/symlink.sh $1 $2; fi
	@vivado -mode $(VIV_MODE) -source $(test_dir)/generate_vivado.tcl -notrace \
		-tclargs --project $1 --sim $(VIV_SIM) --create $(VIV_CREATE) \
		--synth $(VIV_SYNTH) --impl $(VIV_IMPL) --bit $(VIV_BIT) --export $(VIV_EXPORT)
endef
$(foreach module, $(c_modules),$(eval $(call make-sim,$(module),c)))
