################################################################################
# Help
################################################################################

variable script_file
set script_file generate

proc help {} {
  puts "\nDescription:"
  puts "Recreate a Vivado project from this script. The created project will be"
  puts "functionally equivalent to the original project for which this script"
  puts "was generated. The script contains commands for creating a project,"
  puts "filesets, runs, adding/importing sources and setting properties on"
  puts "various objects.\n"
  puts "Syntax:"
  puts "$script_file -tclargs --project <name> \[--sim <type>\]"
  puts "$script_file -tclargs --help\n"
  puts "Usage:"
  puts "Name                   Description"
  puts "---------------------------------------------------------------------"
  puts "--project <name>       The name of the project to create. This must"
  puts "                         must match the name of an existing module"
  puts "--sim <type>           Automatically launch the simulation. Type must"
  puts "                         be one of: behav"
  puts "\[--help\]             Print help information for this script"
  puts "---------------------------------------------------------------------\n"
  exit 0
}

#-------------------------------------------------------------------------------
# Parse Arguments
#-------------------------------------------------------------------------------

variable project_name
variable auto_sim
set auto_sim "0"

if { $::argc > 0 } {
  for {set i 0} {$i < $::argc} {incr i} {
    set option [string trim [lindex $::argv $i]]
    switch -regexp -- $option {
      "--project" { incr i; set project_name [lindex $::argv $i] }
      "--help" { help }
      "--sim" { incr i; set auto_sim [lindex $::argv $i] }
      default {
        puts "ERROR: Unknown option '$option' specified, please type"
        puts "'$script_file -tclargs --help' for usage info.\n"
        return 1
      }
    }
  }
} else {
  puts "Argument error. Use '$script_file -tclargs --help' for usage info.\n"
  return 1
}

switch $auto_sim {
  "0" {set auto_sim 0}
  "behav" {set auto_sim 1}
  default {
    puts "ERROR: Unknown --sim option $auto_sim specified, please type"
    puts "'$script_file -tclargs --help' for usage info.\n"
    return 1
  }
}

################################################################################
# Include
################################################################################

# Defines lremove and listcomp
source ${::env(SHOAL_PATH)}/share/utilities.tcl

################################################################################
# Variables
################################################################################

variable part_name
set part_name xcvu095-ffvc1517-2-e

# Set the reference directory for source file relative paths
set origin_dir ${::env(SHOAL_PATH)}/GAScore/vivado
set src_dir $origin_dir/src/$project_name

# Set the directory path for the original project from where this script was exported
set orig_proj_dir "[file normalize "$origin_dir/projects/$project_name"]"

################################################################################
# Body
################################################################################

# Create project
create_project $project_name $orig_proj_dir -part $part_name -force

# Set the directory path for the new project
set proj_dir [get_property directory [current_project]]

# Set project properties
set obj [get_projects $project_name]
set_property -name "default_lib" -value "xil_defaultlib" -objects $obj
set_property -name "ip_cache_permissions" -value "read write" -objects $obj
set_property -name "ip_output_repo" -value "$proj_dir/$project_name.cache/ip" -objects $obj
set_property -name "part" -value "$part_name" -objects $obj
set_property -name "sim.ip.auto_export_scripts" -value "1" -objects $obj
set_property -name "simulator_language" -value "Mixed" -objects $obj

# Read all source files for this module
set all_files [glob -directory "$src_dir" *]

# Create 'sources_1' fileset (if not found)
if {[string equal [get_filesets -quiet sources_1] ""]} {
  create_fileset -srcset sources_1
}

# Set 'sources_1' fileset object
set obj [get_filesets sources_1]
# remove the tb file from the file set (it's added separately to the simset)
set files [listcomp $all_files [glob -directory "$src_dir" *_tb.sv] ]
# if .tcl files exist, remove them as well
if {! [catch {glob -directory "$src_dir" *.tcl} yikes] } {
  set files [listcomp $files [glob -directory "$src_dir" *.tcl] ]
}
# if waveform config files exist, remove them as well
if {! [catch {glob -directory "$src_dir" *.wcfg} yikes] } {
  set files [listcomp $files [glob -directory "$src_dir" *.wcfg] ]
}
# add the remaining files to the sources set
add_files -norecurse -fileset $obj $files

set data_files [glob -directory "$src_dir" *.dat]

foreach dataFile $data_files {
  set file_obj [get_files -of_objects [get_filesets sources_1] [list "*$dataFile"]]
  set_property -name "file_type" -value "Data Files" -objects $file_obj
}

# Set 'sources_1' fileset properties
set obj [get_filesets sources_1]
set_property -name "top" -value "$project_name" -objects $obj

# Create 'constrs_1' fileset (if not found)
if {[string equal [get_filesets -quiet constrs_1] ""]} {
  create_fileset -constrset constrs_1
}

# Create 'sim_1' fileset (if not found)
if {[string equal [get_filesets -quiet sim_1] ""]} {
  create_fileset -simset sim_1
}

# Set 'sim_1' fileset object
set obj [get_filesets sim_1]
set files [glob -directory "$src_dir" *_tb.sv]
set files [glob -directory "$src_dir" *.wcfg]
add_files -norecurse -fileset $obj $files

# Set 'sim_1' fileset file properties for remote files
foreach dataFile $files {
  set file_obj [get_files -of_objects [get_filesets sim_1] [list "*$dataFile"]]
  set_property -name "file_type" -value "SystemVerilog" -objects $file_obj
}

# Set 'sim_1' fileset properties
set obj [get_filesets sim_1]
set_property -name "top" -value "${project_name}_tb" -objects $obj
set_property -name "xsim.simulate.runtime" -value "-1" -objects $obj
if {! [catch {glob -directory "$src_dir" *.wcfg} yikes] } {
  set_property xsim.view ${project_name}.wcfg $obj
}

if {! [catch {glob -directory "$src_dir" *.tcl} yikes] } {
  set files [glob -directory "$src_dir" *.tcl]
  foreach tclFile $files {
    source $tclFile -notrace
  }
}

# Create 'synth_1' run (if not found)
if {[string equal [get_runs -quiet synth_1] ""]} {
  create_run -name synth_1 -part $part_name -flow {Vivado Synthesis 2017} \
  -strategy "Vivado Synthesis Defaults" -constrset constrs_1
} else {
  set_property strategy "Vivado Synthesis Defaults" [get_runs synth_1]
  set_property flow "Vivado Synthesis 2017" [get_runs synth_1]
}
set obj [get_runs synth_1]
set_property -name "needs_refresh" -value "1" -objects $obj
set_property -name "part" -value "$part_name" -objects $obj

# set the current synth run
current_run -synthesis [get_runs synth_1]

# Create 'impl_1' run (if not found)
if {[string equal [get_runs -quiet impl_1] ""]} {
  create_run -name impl_1 -part $part_name -flow {Vivado Implementation 2017} \
    -strategy "Vivado Implementation Defaults" -constrset constrs_1 \
    -parent_run synth_1
} else {
  set_property strategy "Vivado Implementation Defaults" [get_runs impl_1]
  set_property flow "Vivado Implementation 2017" [get_runs impl_1]
}
set obj [get_runs impl_1]
set_property -name "part" -value "$part_name" -objects $obj
set_property -name "steps.write_bitstream.args.readback_file" -value "0" \
  -objects $obj
set_property -name "steps.write_bitstream.args.verbose" -value "0" \
  -objects $obj

# set the current impl run
current_run -implementation [get_runs impl_1]

if {$auto_sim == 1} {
  launch_simulation
}
