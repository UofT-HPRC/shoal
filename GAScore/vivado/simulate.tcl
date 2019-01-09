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

open_project -part $part_name $orig_proj_dir/$project_name.xpr

upgrade_ip [get_ips]

reset_target simulation [get_files GAcore_bd.bd]
export_ip_user_files -of_objects  [get_files GAScore_bd.bd] -sync -no_script -force -quiet

if {$auto_sim == 1} {
  launch_simulation
}