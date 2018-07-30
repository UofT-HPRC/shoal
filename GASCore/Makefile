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

ifeq ($(SHOAL_PATH),)
	$(error SHOAL_PATH not set in env -- must be set to the absolute path of \
	of the repository root)
endif

local_dir = $(SHOAL_PATH)/GASCore
obj_dir = $(local_dir)/build
bin_dir = $(local_dir)/build/bin
src_dir = $(local_dir)/src
test_dir = $(local_dir)/testbench
include_dir = $(SHOAL_PATH)/GASCore/include

share_dir = $(SHOAL_PATH)/share
share_obj_dir = $(share_dir)/build
share_bin_dir = $(share_dir)/build/bin
share_src_dir = $(share_dir)/src
share_include_dir = $(SHOAL_PATH)/share/include

src = $(shell find $(src_dir) -name '*.cpp' -printf '%f\n' | \
sort -k 1nr | cut -f2-)
share_src = $(shell find $(share_src_dir) -name '*.cpp' -printf '%f\n' | \
sort -k 1nr | cut -f2-)
dep = $(src:%.cpp=$(obj_dir)/%.d)
dep += $(share_src:%.cpp=$(share_obj_dir)/%.d)

helper_objs = $(share_obj_dir)/globals.o $(obj_dir)/packet_defs.o

CC = g++
CFLAGS  = -g -Wall -I$(include_dir) -I$(share_include_dir) \
	-I/media/sharm294/HDD_1TB/Xilinx/Vivado_HLS/2017.2/include \
	-Wno-unknown-pragmas -Wno-comment -MMD -MP

###############################################################################
# Body
###############################################################################

.PHONY: init test test-v1 clean

#------------------------------------------------------------------------------
# Main
#------------------------------------------------------------------------------

init:
	./init.sh

test: $(bin_dir)/am_rx_tb
	python $(test_dir)/am_rx_generate.py
	$(bin_dir)/am_rx_tb

test-v1: $(bin_dir)/am_rx_tb
	python $(test_dir)/am_rx_generate.py
	$(bin_dir)/am_rx_tb -v

#------------------------------------------------------------------------------
# Executables
#------------------------------------------------------------------------------

$(bin_dir)/am_rx_tb: $(obj_dir)/am_rx_tb.o $(obj_dir)/am_rx.o $(helper_objs)
	$(CC) $(CFLAGS) -o $(bin_dir)/am_rx_tb $(obj_dir)/am_rx_tb.o \
	$(obj_dir)/am_rx.o $(helper_objs)

#------------------------------------------------------------------------------
# Object Files
#------------------------------------------------------------------------------

$(obj_dir)/am_rx_tb.o: $(test_dir)/am_rx_tb.cpp
	$(CC) $(CFLAGS) -o $(obj_dir)/am_rx_tb.o -c $(test_dir)/am_rx_tb.cpp

$(obj_dir)/am_rx.o: $(src_dir)/am_rx.cpp
	$(CC) $(CFLAGS) -o $(obj_dir)/am_rx.o -c $(src_dir)/am_rx.cpp

$(obj_dir)/packet_defs.o: $(src_dir)/packet_defs.cpp
	$(CC) $(CFLAGS) -o $(obj_dir)/packet_defs.o -c $(src_dir)/packet_defs.cpp

$(share_obj_dir)/globals.o: $(share_src_dir)/globals.cpp
	$(CC) $(CFLAGS) -o $(share_obj_dir)/globals.o \
	-c $(share_src_dir)/globals.cpp

-include $(dep)

#------------------------------------------------------------------------------
# Cleanup
#------------------------------------------------------------------------------

clean: 
	@$(RM) $(obj_dir)/*.o $(obj_dir)/*.d $(bin_dir)/* \
	$(share_obj_dir)/*.o $(share_obj_dir)/*.d $(share_bin_dir)/*