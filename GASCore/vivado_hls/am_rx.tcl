### ///////////////////////////////////////////////////////////////////////////////
### // Copyright (c) 2015 Xilinx, Inc.
### // All Rights Reserved
### ///////////////////////////////////////////////////////////////////////////////
### //   ____  ____
### //  /   /\/   /
### // /___/  \  /    Vendor: Xilinx
### // \   \   \/     Version: 1.0
### //  \   \         Application : Vivado HLS
### //  /   /         Filename: run.tcl
### // /___/   /\     Timestamp: Wed Mar 30 5:00:00 PST 2016
### // \   \  /  \
### //  \___\/\___\
### //
### //Command: vivado_hls run.tcl
### //Device:  xcku035-fbva676-1-i
### //Design Name: diff_sq_acc
### //Purpose:
### //    This file is the script to 
### //    1) do C++ functional simulation
### //    2) synthesize C++ into RTL 
### //    3) do functional verificaiton for the generated RTL
### //    4) synthesize the RTL with Vivado to check actual resource utilization
### //       and block-level timing performance
### //    
### ///////////////////////////////////////////////////////////////////////////////

### default setting
set Project     am_rx
set Solution    Virtex_Ultrascale
set Device      "xcvu095-ffvc1517-2-e"
set Flow        ""
set Clock       4.0
set DefaultFlag 1

set src_dir ${::env(SHOAL_PATH)}/GASCore/src
set test_dir ${::env(SHOAL_PATH)}/GASCore/testbench
set include -I${::env(SHOAL_PATH)}/GASCore/include

#### main part

# Project settings
open_project $Project -reset

# Add the file for synthesis
add_files $src_dir/am_rx.cpp -cflags $include
add_files $src_dir/globals.cpp -cflags $include
add_files $src_dir/packet_defs.cpp -cflags $include

# Add testbench files for co-simulation
add_files -tb  $test_dir/am_rx_tb.cpp -cflags $include

# Set top module of the design
set_top am_rx

# Solution settings
open_solution -reset $Solution

# Add library
set_part $Device

# Set the target clock period
create_clock -period $Clock

#################
# C SIMULATION
#################
csim_design

#############
# SYNTHESIS #
#############
csynth_design

#################
# CO-SIMULATION #
#################
#cosim_design -rtl verilog -trace_level all

##################
# IMPLEMENTATION #
##################
export_design -format ipxact


exit
