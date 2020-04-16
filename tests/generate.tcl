################################################################################
# Parse Arguments
################################################################################

if { $::argc > 2 } {
  for {set i 2} {$i < $::argc} {incr i} {
    if {$i == 2} {
      set Project [lindex $::argv $i]
    }
    if {$i == 3} {
      set top [lindex $::argv $i]
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

set src_dir ${::env(SHOAL_PATH)}/src
set test_dir ${::env(SHOAL_PATH)}/tests
set local_include -I${::env(SHOAL_PATH)}/GAScore/include
set global_include -I${::env(SHOAL_PATH)}/include
set galapagos_include -I${::env(GALAPAGOS_PATH)}/middleware/libGalapagos
set galapagos_library -I${::env(GALAPAGOS_PATH)}/middleware/include
set spdlog_library -I${::env(GALAPAGOS_PATH)}/util/spdlog/include
append include -D__HLS__ " " -std=c++11 " " $local_include " " $global_include
append include " " $galapagos_include " " $galapagos_library " " $spdlog_library

################################################################################
# Body
################################################################################

# Project settings
open_project $Project -reset

# Add the file for synthesis
puts $include
add_files $src_dir/active_messages.cpp -cflags $include
add_files $src_dir/shoal_kernel.cpp -cflags $include
add_files $test_dir/$top.cpp -cflags $include

# Add testbench files for co-simulation
# add_files -tb  $test_dir/${Project}/${Project}_tb.cpp -cflags $include

# Set top module of the design
set_top $top

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
