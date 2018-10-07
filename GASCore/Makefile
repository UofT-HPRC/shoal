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

ifndef SONAR_PATH
$(error SONAR_PATH not set in env -- sonar is required for testing)
endif

ifndef SHOAL_VIVADO_HLS
$(error SHOAL_VIVADO_HLS not set in env -- must be set to the absolute path of \
of the Vivado HLS include/ directory. Did you source init.sh?)
endif

local_dir = $(SHOAL_PATH)/GASCore
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

obj = $(shell find $(obj_dir) -name '*.o' -printf '%f\n' | \
sort -k 1nr | cut -f2-)
share_obj = $(shell find $(share_obj_dir) -name '*.o' -printf '%f\n' | \
sort -k 1nr | cut -f2-)
dep = $(obj:%.o=$(obj_dir)/%.d)
dep += $(share_obj:%.o=$(share_obj_dir)/%.d)

CC = g++
CFLAGS = -g -Wall -I$(include_dir) -I$(share_include_dir) \
	-I$(SHOAL_VIVADO_HLS) \
	-Wno-unknown-pragmas -Wno-comment -MMD -MP

###############################################################################
# Body
###############################################################################

.PHONY: test hw sim clean yaml

#------------------------------------------------------------------------------
# Main
#------------------------------------------------------------------------------

test: yaml $(bin_dir)/am_rx_tb # $(bin_dir)/xpams_tb $(bin_dir)/am_tx_tb
	@# python $(SONAR_PATH)/sonar.py env SHOAL_PATH /GASCore/testbench/am_rx.yaml
	@# python $(share_testbench_dir)/generate.py env SHOAL_PATH /GASCore/testbench/xpams.json
	@# python $(share_testbench_dir)/generate.py env SHOAL_PATH /GASCore/testbench/am_tx.json
	$(bin_dir)/am_rx_tb --printState
	@# $(bin_dir)/xpams_tb
	@# $(bin_dir)/am_tx_tb

yaml:
	python $(SONAR_PATH)/sonar.py env SHOAL_PATH /GASCore/testbench/am_rx.yaml

hw:
	@$(hls_dir)/am_rx.sh
	# $(hls_dir)/am_tx.sh
	# $(hls_dir)/xpams.sh

sim:
	@$(vivado_dir)/am_rx.sh

#------------------------------------------------------------------------------
# Executables
#------------------------------------------------------------------------------

$(bin_dir)/am_rx_tb: $(obj_dir)/am_rx_tb.o $(obj_dir)/am_rx.o $(obj_dir)/gascore.o
	$(CC) $(CFLAGS) -o $(bin_dir)/am_rx_tb $(obj_dir)/am_rx_tb.o \
	$(obj_dir)/am_rx.o $(obj_dir)/gascore.o

$(bin_dir)/xpams_tb: $(obj_dir)/xpams_tb.o $(obj_dir)/xpams.o $(obj_dir)/gascore.o
	$(CC) $(CFLAGS) -o $(bin_dir)/xpams_tb $(obj_dir)/xpams_tb.o \
	$(obj_dir)/xpams.o $(obj_dir)/gascore.o

$(bin_dir)/am_tx_tb: $(obj_dir)/am_tx_tb.o $(obj_dir)/am_tx.o $(obj_dir)/gascore.o
	$(CC) $(CFLAGS) -o $(bin_dir)/am_tx_tb $(obj_dir)/am_tx_tb.o \
	$(obj_dir)/am_tx.o $(obj_dir)/gascore.o

#------------------------------------------------------------------------------
# Object Files
#------------------------------------------------------------------------------

$(obj_dir)/am_rx_tb.o: $(test_build_dir)/am_rx_tb.cpp
	$(CC) $(CFLAGS) -o $(obj_dir)/am_rx_tb.o -c $(test_build_dir)/am_rx_tb.cpp

$(obj_dir)/xpams_tb.o: $(test_build_dir)/xpams_tb.cpp
	$(CC) $(CFLAGS) -o $(obj_dir)/xpams_tb.o -c $(test_build_dir)/xpams_tb.cpp

$(obj_dir)/am_tx_tb.o: $(test_build_dir)/am_tx_tb.cpp
	$(CC) $(CFLAGS) -o $(obj_dir)/am_tx_tb.o -c $(test_build_dir)/am_tx_tb.cpp

$(obj_dir)/am_rx.o: $(src_dir)/am_rx.cpp
	$(CC) $(CFLAGS) -o $(obj_dir)/am_rx.o -c $(src_dir)/am_rx.cpp

$(obj_dir)/xpams.o: $(src_dir)/xpams.cpp
	$(CC) $(CFLAGS) -o $(obj_dir)/xpams.o -c $(src_dir)/xpams.cpp

$(obj_dir)/am_tx.o: $(src_dir)/am_tx.cpp
	$(CC) $(CFLAGS) -o $(obj_dir)/am_tx.o -c $(src_dir)/am_tx.cpp

$(obj_dir)/gascore.o: $(src_dir)/gascore.cpp
	$(CC) $(CFLAGS) -o $(obj_dir)/gascore.o -c $(src_dir)/gascore.cpp

-include $(dep)

#------------------------------------------------------------------------------
# Cleanup
#------------------------------------------------------------------------------

clean: 
	@$(RM) $(obj_dir)/*.o $(obj_dir)/*.d $(bin_dir)/* \
	$(share_obj_dir)/*.o $(share_obj_dir)/*.d $(share_bin_dir)/* \
	$(vivado_dir)/src/*