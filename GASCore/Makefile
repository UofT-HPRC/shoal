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

ifeq ($(SHOAL_VIVADO_HLS),)
	$(error SHOAL_VIVADO_HLS not set in env -- must be set to the absolute path of \
	of the Vivado HLS include/ directory)
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

# helper_objs = $(share_obj_dir)/st.o $(obj_dir)/packet_defs.o

CC = g++
CFLAGS = -g -Wall -I$(include_dir) -I$(share_include_dir) \
	-I$(SHOAL_VIVADO_HLS) \
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

test: $(bin_dir)/am_rx_tb $(bin_dir)/xpams_tb
	python $(test_dir)/generate.py am_rx
	python $(test_dir)/generate.py xpams
	$(bin_dir)/am_rx_tb
	$(bin_dir)/xpams_tb

test-v1: $(bin_dir)/am_rx_tb
	python $(test_dir)/am_rx_generate.py
	$(bin_dir)/am_rx_tb -v

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

$(obj_dir)/am_rx_tb.o: $(test_dir)/am_rx_tb.cpp
	$(CC) $(CFLAGS) -o $(obj_dir)/am_rx_tb.o -c $(test_dir)/am_rx_tb.cpp

$(obj_dir)/xpams_tb.o: $(test_dir)/xpams_tb.cpp
	$(CC) $(CFLAGS) -o $(obj_dir)/xpams_tb.o -c $(test_dir)/xpams_tb.cpp

$(obj_dir)/am_tx_tb.o: $(test_dir)/am_tx_tb.cpp
	$(CC) $(CFLAGS) -o $(obj_dir)/am_tx_tb.o -c $(test_dir)/am_tx_tb.cpp

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
	$(share_obj_dir)/*.o $(share_obj_dir)/*.d $(share_bin_dir)/*
