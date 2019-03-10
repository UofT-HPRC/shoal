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

ifndef SHOAL_HLS_PATH
$(error SHOAL_HLS_PATH not set in env -- must be set to the absolute path of \
of the Vivado HLS include/ directory. Did you source init.sh?)
endif

include_dir=$(SHOAL_PATH)/include
GAScore_dir=$(SHOAL_PATH)/GAScore/include
share_dir=$(SHOAL_PATH)/share/include

CC = g++
CFLAGS = -g -Wall -O0 -I$(include_dir) -I$(GAScore_dir) -I$(share_dir) -I$(SHOAL_HLS_PATH)\
	-Wno-unused-value -Wno-unused-variable -Wno-comment \
	-Wno-unused-but-set-variable -Wno-unused-function -MMD -MP -pthread -std=c++11

###############################################################################
# Body
###############################################################################

.PHONY: init clean

#------------------------------------------------------------------------------
# Main
#------------------------------------------------------------------------------

init:
	./init.sh

libs:
	$(CC) -o src/commtest_gascorev2 $(CFLAGS) src/the_gasnet_core.cpp src/platforms.cpp src/main_wrapper.cpp src/the_gasnet_globals.cpp \
	$(SHOAL_PATH)/GAScore/src/utilities.cpp src/active_messages.cpp tests/commtest_gascorev2.cpp -Wl,--wrap=main -lpthread -lrt 

run:
	./src/commtest_gascorev2 --tgn_myip=192.168.1.103 --tgn_config=./src/gasnet_config --tgn_nopin

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
	@echo "Run 'make clean-x' where x is a level"
	@echo "Make Clean levels:"
	@echo "    1. Removes junk (e.g. Vivado log files etc.)"

clean-1:
	@find . -name "vivado*.log" -type f -delete
	@find . -name "vivado*.jou" -type f -delete
	@rm -rf `find -type d -name .Xil`
	@find . -name "vivado_pid*.str" -type f -delete
	@find . -name "hs_err_pid*.log" -type f -delete

purge:
	@rm -rf ~/.shoal
	@sed -i '/added by shoal/d' ~/.bashrc
