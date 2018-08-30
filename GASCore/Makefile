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
include_dir = $(local_dir)/include
hls_dir = $(local_dir)/vivado_hls

share_dir = $(SHOAL_PATH)/share
share_obj_dir = $(share_dir)/build
share_bin_dir = $(share_dir)/build/bin
share_src_dir = $(share_dir)/src
share_include_dir = $(SHOAL_PATH)/share/include
share_testbench_dir = $(share_dir)/testbench

obj = $(shell find $(obj_dir) -name '*.o' -printf '%f\n' | \
sort -k 1nr | cut -f2-)
share_obj = $(shell find $(share_obj_dir) -name '*.o' -printf '%f\n' | \
sort -k 1nr | cut -f2-)
dep = $(obj:%.o=$(obj_dir)/%.d)
dep += $(share_obj:%.o=$(share_obj_dir)/%.d)

# helper_objs = $(share_obj_dir)/st.o $(obj_dir)/packet_defs.o

CC = g++
CFLAGS = -g -Wall -I$(include_dir) -I$(share_include_dir) \
	-I$(SHOAL_VIVADO_HLS) \
	-Wno-unknown-pragmas -Wno-comment -MMD -MP

###############################################################################
# Body
###############################################################################

.PHONY: init test sample hw clean

#------------------------------------------------------------------------------
# Main
#------------------------------------------------------------------------------

init:
	./init.sh

test: $(bin_dir)/am_rx_tb $(bin_dir)/xpams_tb $(bin_dir)/am_tx_tb
	python $(share_testbench_dir)/generate.py 0 /GASCore/testbench/am_rx.json
	python $(share_testbench_dir)/generate.py 0 /GASCore/testbench/xpams.json
	python $(share_testbench_dir)/generate.py 0 /GASCore/testbench/am_tx.json
	$(bin_dir)/am_rx_tb
	$(bin_dir)/xpams_tb
	$(bin_dir)/am_tx_tb

sample: $(bin_dir)/sample_tb
	python $(share_testbench_dir)/generate.py 0 /GASCore/testbench/sample.json
	$(bin_dir)/sample_tb

hw:
	$(hls_dir)/am_rx.sh
	$(hls_dir)/am_tx.sh
	$(hls_dir)/xpams.sh

#------------------------------------------------------------------------------
# Executables
#------------------------------------------------------------------------------

$(bin_dir)/sample_tb: $(obj_dir)/sample_tb.o $(obj_dir)/sample.o
	$(CC) $(CFLAGS) -o $(bin_dir)/sample_tb $(obj_dir)/sample_tb.o \
	$(obj_dir)/sample.o

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

$(obj_dir)/sample_tb.o: $(test_dir)/sample_tb.cpp
	$(CC) $(CFLAGS) -o $(obj_dir)/sample_tb.o -c $(test_dir)/sample_tb.cpp

$(obj_dir)/am_rx.o: $(src_dir)/am_rx.cpp
	$(CC) $(CFLAGS) -o $(obj_dir)/am_rx.o -c $(src_dir)/am_rx.cpp

$(obj_dir)/xpams.o: $(src_dir)/xpams.cpp
	$(CC) $(CFLAGS) -o $(obj_dir)/xpams.o -c $(src_dir)/xpams.cpp

$(obj_dir)/am_tx.o: $(src_dir)/am_tx.cpp
	$(CC) $(CFLAGS) -o $(obj_dir)/am_tx.o -c $(src_dir)/am_tx.cpp

$(obj_dir)/sample.o: $(src_dir)/sample.cpp
	$(CC) $(CFLAGS) -o $(obj_dir)/sample.o -c $(src_dir)/sample.cpp
	
$(obj_dir)/gascore.o: $(src_dir)/gascore.cpp
	$(CC) $(CFLAGS) -o $(obj_dir)/gascore.o -c $(src_dir)/gascore.cpp

-include $(dep)

#------------------------------------------------------------------------------
# Cleanup
#------------------------------------------------------------------------------

clean: 
	@$(RM) $(obj_dir)/*.o $(obj_dir)/*.d $(bin_dir)/* \
	$(share_obj_dir)/*.o $(share_obj_dir)/*.d $(share_bin_dir)/*
