###############################################################################
# Include
###############################################################################

###############################################################################
# Prologue
###############################################################################

MAKEFLAGS += --warn-undefined-variables
SHELL := bash
.SHELLFLAGS := -eu -o pipefail -c
.DEFAULT_GOAL := test
.DELETE_ON_ERROR:
.SUFFIXES:

###############################################################################
# Variables
###############################################################################

ifndef SHOAL_PATH
$(error SHOAL_PATH not set in env -- must be set to the absolute path of \
of the repository root. Did you source init.sh?)
endif

ifndef SHOAL_SHARE_PATH
$(error SHOAL_SHARE_PATH not set in env -- must be set to the absolute path of \
of the share repository root. Did you source init.sh?)
endif

ifndef SHOAL_VIVADO_HLS
$(error SHOAL_VIVADO_HLS not set in env -- must be set to the absolute path of \
of the Vivado HLS include/ directory. Did you source init.sh?)
endif

CC = g++
CFLAGS = -g -Wall -I$(include_dir) -I$(share_include_dir) \
	-I$(SHOAL_VIVADO_HLS) \
	-Wno-unknown-pragmas -Wno-comment -MMD -MP

###############################################################################
# Body
###############################################################################

.PHONY: init clean

#------------------------------------------------------------------------------
# Main
#------------------------------------------------------------------------------

init:
	./init.sh

#------------------------------------------------------------------------------
# Executables
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Object Files
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Cleanup
#------------------------------------------------------------------------------

clean:
	@$(RM)

purge:
	@rm -rf ~/.shoal
	@sed -i '/added by shoal/d' ~/.bashrc