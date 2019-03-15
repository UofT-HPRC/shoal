namespace eval 2017_2 {

proc create_crossbars {} {
  create_ip -name axi_crossbar -vendor xilinx.com -library ip -version 2.1 -module_name axi_crossbar_2
  set_property -dict [list \
    CONFIG.NUM_SI {2} \
    CONFIG.NUM_MI {1} \
    CONFIG.DATA_WIDTH {64} \
    CONFIG.M00_A00_ADDR_WIDTH {32} \
  ] [get_ips axi_crossbar_2]

  create_ip -name axi_crossbar -vendor xilinx.com -library ip -version 2.1 -module_name axi_crossbar_3
  set_property -dict [list \
    CONFIG.NUM_SI {3} \
    CONFIG.NUM_MI {1} \
    CONFIG.DATA_WIDTH {64} \
    CONFIG.M00_A00_ADDR_WIDTH {32} \
  ] [get_ips axi_crossbar_3]

  create_ip -name axi_crossbar -vendor xilinx.com -library ip -version 2.1 -module_name axi_crossbar_4
  set_property -dict [list \
    CONFIG.NUM_SI {4} \
    CONFIG.NUM_MI {1} \
    CONFIG.DATA_WIDTH {64} \
    CONFIG.M00_A00_ADDR_WIDTH {32} \
  ] [get_ips axi_crossbar_4]

  create_ip -name axi_crossbar -vendor xilinx.com -library ip -version 2.1 -module_name axi_crossbar_5
  set_property -dict [list \
    CONFIG.NUM_SI {5} \
    CONFIG.NUM_MI {1} \
    CONFIG.DATA_WIDTH {64} \
    CONFIG.M00_A00_ADDR_WIDTH {32} \
  ] [get_ips axi_crossbar_5]

  create_ip -name axi_crossbar -vendor xilinx.com -library ip -version 2.1 -module_name axi_crossbar_6
  set_property -dict [list \
    CONFIG.NUM_SI {6} \
    CONFIG.NUM_MI {1} \
    CONFIG.DATA_WIDTH {64} \
    CONFIG.M00_A00_ADDR_WIDTH {32} \
  ] [get_ips axi_crossbar_6]

  create_ip -name axi_crossbar -vendor xilinx.com -library ip -version 2.1 -module_name axi_crossbar_7
  set_property -dict [list \
    CONFIG.NUM_SI {7} \
    CONFIG.NUM_MI {1} \
    CONFIG.DATA_WIDTH {64} \
    CONFIG.M00_A00_ADDR_WIDTH {32} \
  ] [get_ips axi_crossbar_7]

  create_ip -name axi_crossbar -vendor xilinx.com -library ip -version 2.1 -module_name axi_crossbar_8
  set_property -dict [list \
    CONFIG.NUM_SI {8} \
    CONFIG.NUM_MI {1} \
    CONFIG.DATA_WIDTH {64} \
    CONFIG.M00_A00_ADDR_WIDTH {32} \
  ] [get_ips axi_crossbar_8]

  create_ip -name axi_crossbar -vendor xilinx.com -library ip -version 2.1 -module_name axi_crossbar_9
  set_property -dict [list \
    CONFIG.NUM_SI {9} \
    CONFIG.NUM_MI {1} \
    CONFIG.DATA_WIDTH {64} \
    CONFIG.M00_A00_ADDR_WIDTH {32} \
  ] [get_ips axi_crossbar_9]

  create_ip -name axi_crossbar -vendor xilinx.com -library ip -version 2.1 -module_name axi_crossbar_10
  set_property -dict [list \
    CONFIG.NUM_SI {10} \
    CONFIG.NUM_MI {1} \
    CONFIG.DATA_WIDTH {64} \
    CONFIG.M00_A00_ADDR_WIDTH {32} \
  ] [get_ips axi_crossbar_10]

  create_ip -name axi_crossbar -vendor xilinx.com -library ip -version 2.1 -module_name axi_crossbar_11
  set_property -dict [list \
    CONFIG.NUM_SI {11} \
    CONFIG.NUM_MI {1} \
    CONFIG.DATA_WIDTH {64} \
    CONFIG.M00_A00_ADDR_WIDTH {32} \
  ] [get_ips axi_crossbar_11]

  create_ip -name axi_crossbar -vendor xilinx.com -library ip -version 2.1 -module_name axi_crossbar_12
  set_property -dict [list \
    CONFIG.NUM_SI {12} \
    CONFIG.NUM_MI {1} \
    CONFIG.DATA_WIDTH {64} \
    CONFIG.M00_A00_ADDR_WIDTH {32} \
  ] [get_ips axi_crossbar_12]

  create_ip -name axi_crossbar -vendor xilinx.com -library ip -version 2.1 -module_name axi_crossbar_13
  set_property -dict [list \
    CONFIG.NUM_SI {13} \
    CONFIG.NUM_MI {1} \
    CONFIG.DATA_WIDTH {64} \
    CONFIG.M00_A00_ADDR_WIDTH {32} \
  ] [get_ips axi_crossbar_13]

  create_ip -name axi_crossbar -vendor xilinx.com -library ip -version 2.1 -module_name axi_crossbar_14
  set_property -dict [list \
    CONFIG.NUM_SI {14} \
    CONFIG.NUM_MI {1} \
    CONFIG.DATA_WIDTH {64} \
    CONFIG.M00_A00_ADDR_WIDTH {32} \
  ] [get_ips axi_crossbar_14]

  create_ip -name axi_crossbar -vendor xilinx.com -library ip -version 2.1 -module_name axi_crossbar_15
  set_property -dict [list \
    CONFIG.NUM_SI {15} \
    CONFIG.NUM_MI {1} \
    CONFIG.DATA_WIDTH {64} \
    CONFIG.M00_A00_ADDR_WIDTH {32} \
  ] [get_ips axi_crossbar_15]

  create_ip -name axi_crossbar -vendor xilinx.com -library ip -version 2.1 -module_name axi_crossbar_16
  set_property -dict [list \
    CONFIG.NUM_SI {16} \
    CONFIG.NUM_MI {1} \
    CONFIG.DATA_WIDTH {64} \
    CONFIG.M00_A00_ADDR_WIDTH {32} \
  ] [get_ips axi_crossbar_16]
}

}

# defines get_design_name
source ${::env(SHOAL_PATH)}/helper/utilities.tcl

set current_vivado_version [version -short]

if { [string first 2017.2 $current_vivado_version] != -1 } {
  2017_2::create_crossbars
} else {
  puts ""
  catch {common::send_msg_id "BD_TCL-109" "ERROR" "Unsupported Vivado version:\
    $current_vivado_version for generating the GAScore"}

  return 1
}
