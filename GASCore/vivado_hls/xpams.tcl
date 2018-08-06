### default setting
set Project     xpams
set Solution    Virtex_Ultrascale
set Device      "xcvu095-ffvc1517-2-e"
set Flow        ""
set Clock       4.0
set DefaultFlag 1

set src_dir ${::env(SHOAL_PATH)}/GASCore/src
set test_dir ${::env(SHOAL_PATH)}/GASCore/testbench
set local_include -I${::env(SHOAL_PATH)}/GASCore/include
set share_src_dir ${::env(SHOAL_PATH)}/share/src
set share_include -I${::env(SHOAL_PATH)}/share/include
append include $local_include " " $share_include

#### main part

# Project settings
open_project $Project -reset

# Add the file for synthesis
add_files $src_dir/xpams.cpp -cflags $include
add_files $src_dir/gascore.cpp -cflags $include

# Add testbench files for co-simulation
#add_files -tb  $test_dir/am_rx_tb.cpp -cflags $include

# Set top module of the design
set_top xpams

# Solution settings
open_solution -reset $Solution

# Add library
set_part $Device

# Set the target clock period
create_clock -period $Clock

#################
# C SIMULATION
#################
#csim_design

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
#export_design -format ipxact


exit
