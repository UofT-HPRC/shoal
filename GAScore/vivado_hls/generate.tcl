################################################################################
# Parse Arguments
################################################################################

if { $::argc > 2 } {
  for {set i 2} {$i < $::argc} {incr i} {
      if {$i == 2} {
        set Project [lindex $::argv $i]
      }
  }
} else {
  puts "Argument error. The project name must be an argument\n"
  exit
}

################################################################################
# Variables
################################################################################

set Solution ${::env(SHOAL_PART_FAMILY)}
set Device ${::env(SHOAL_PART)}
set Flow ""
set Clock 4.0
set DefaultFlag 1

set src_dir ${::env(SHOAL_PATH)}/GAScore/src
set test_dir ${::env(SHOAL_PATH)}/GAScore/testbench/build
set local_include -I${::env(SHOAL_PATH)}/GAScore/include
set share_src_dir ${::env(SHOAL_PATH)}/share/src
set share_include -I${::env(SHOAL_PATH)}/share/include
append include $local_include " " $share_include

################################################################################
# Body
################################################################################

# Project settings
open_project $Project -reset

# Add the file for synthesis
add_files $src_dir/$Project.cpp -cflags $include
add_files $src_dir/utilities.cpp -cflags $include

# Add testbench files for co-simulation
add_files -tb  $test_dir/${Project}/${Project}_tb.cpp -cflags $include

# Set top module of the design
set_top $Project

# Solution settings
open_solution -reset $Solution

# Set module prefix
config_rtl -encoding auto -prefix ${Project}_ -reset all -reset_level low

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
export_design -format ipxact

exit
