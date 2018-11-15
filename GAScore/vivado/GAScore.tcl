proc get_design_name {design_name} {
  set cur_design [current_bd_design -quiet]
  set list_cells [get_bd_cells -quiet]
  set errMsg ""
  set nRet 0

  if { ${design_name} eq "" } {
    # USE CASES:
    # 1): Missing design name
    set errMsg "Please set the variable <design_name> to a non-empty value."
    set nRet 1
  } elseif { ${cur_design} ne "" && ${list_cells} eq "" } {
    # USE CASES:
    # 2): Current design opened AND is empty AND names same.
    # 3): Current design opened AND is empty AND names diff; design_name NOT 
    #     in project.
    # 4): Current design opened AND is empty AND names diff; design_name exists
    #     in project.

    if { $cur_design ne $design_name } {
      common::send_msg_id "BD_TCL-001" "INFO" "Changing value of \
        <design_name> from <$design_name> to <$cur_design> since current \
        design is empty."
      set design_name [get_property NAME $cur_design]
    }
    common::send_msg_id "BD_TCL-002" "INFO" "Constructing design in IPI design \
      <$cur_design>..."

  } elseif { ${cur_design} ne "" && $list_cells ne "" && \
    $cur_design eq $design_name } {
    # USE CASES:
    # 5) Current design opened AND has components AND same names.

    set errMsg "Design <$design_name> already exists in your project, please \
      set the variable <design_name> to another value."
    set nRet 1
  } elseif { [get_files -quiet ${design_name}.bd] ne "" } {
    # USE CASES: 
    # 6) Current opened design, has components, but diff names, design_name 
    #    exists in project.
    # 7) No opened design, design_name exists in project.
    set errMsg "Design <$design_name> already exists in your project, please \
      set the variable <design_name> to another value."
    set nRet 2
  } else {
    # USE CASES:
    #    8) No opened design, design_name not in project.
    #    9) Current opened design, has components, but diff names, design_name 
    #       not in project.
    common::send_msg_id "BD_TCL-003" "INFO" "Currently there is no design \
      <$design_name> in project, so creating one..."

    create_bd_design $design_name

    common::send_msg_id "BD_TCL-004" "INFO" "Making design <$design_name> as \
      current_bd_design."
    current_bd_design $design_name
  }

  return [list $cur_design $design_name $errMsg $nRet]
}

namespace eval 2017_2 {

proc create_GAScore {bd_name} {
  
  set design_name $bd_name
  
  set result [get_design_name $design_name]

  set cur_design [lindex $result 0]
  set design_name [lindex $result 1]
  set errMsg [lindex $result 2]
  set nRet [lindex $result 3]

  if { $nRet != 0 } {
    catch {common::send_msg_id "BD_TCL-114" "ERROR" $errMsg}
    return $nRet
  }

  set parentCell [get_bd_cells /]

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
    catch {common::send_msg_id "BD_TCL-100" "ERROR" "Unable to find parent 
      cell <$parentCell>!"}
    return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
    catch {common::send_msg_id "BD_TCL-101" "ERROR" "Parent <$parentObj> has \
      TYPE = <$parentType>. Expected to be <hier>."}
    return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj


  # Create interface ports
  set axi_mem [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 axi_mem ]
  set_property -dict [ list \
    CONFIG.ADDR_WIDTH {32} \
    CONFIG.DATA_WIDTH {32} \
    CONFIG.NUM_READ_OUTSTANDING {2} \
    CONFIG.NUM_WRITE_OUTSTANDING {2} \
    CONFIG.PROTOCOL {AXI4} \
  ] $axi_mem
  set axis_handler [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:axis_rtl:1.0 axis_handler ]
  set axis_kernel_in [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:axis_rtl:1.0 axis_kernel_in ]
  set_property -dict [ list \
    CONFIG.HAS_TKEEP {1} \
    CONFIG.HAS_TLAST {1} \
    CONFIG.HAS_TREADY {1} \
    CONFIG.HAS_TSTRB {0} \
    CONFIG.LAYERED_METADATA {xilinx.com:interface:datatypes:1.0 {TDATA {datatype {name {attribs {resolve_type immediate dependency {} format string minimum {} maximum {}} value {}} bitwidth {attribs {resolve_type immediate dependency {} format long minimum {} maximum {}} value 64} bitoffset {attribs {resolve_type immediate dependency {} format long minimum {} maximum {}} value 0} integer {signed {attribs {resolve_type immediate dependency {} format bool minimum {} maximum {}} value false}}}}}} \
    CONFIG.TDATA_NUM_BYTES {8} \
    CONFIG.TDEST_WIDTH {0} \
    CONFIG.TID_WIDTH {0} \
    CONFIG.TUSER_WIDTH {0} \
  ] $axis_kernel_in
  set axis_kernel_out [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:axis_rtl:1.0 axis_kernel_out ]
  set axis_net_in [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:axis_rtl:1.0 axis_net_in ]
  set_property -dict [ list \
    CONFIG.HAS_TKEEP {1} \
    CONFIG.HAS_TLAST {1} \
    CONFIG.HAS_TREADY {1} \
    CONFIG.HAS_TSTRB {0} \
    CONFIG.LAYERED_METADATA {xilinx.com:interface:datatypes:1.0 {TDATA {datatype {name {attribs {resolve_type immediate dependency {} format string minimum {} maximum {}} value {}} bitwidth {attribs {resolve_type immediate dependency {} format long minimum {} maximum {}} value 64} bitoffset {attribs {resolve_type immediate dependency {} format long minimum {} maximum {}} value 0} integer {signed {attribs {resolve_type immediate dependency {} format bool minimum {} maximum {}} value false}}}}}} \
    CONFIG.TDATA_NUM_BYTES {8} \
    CONFIG.TDEST_WIDTH {0} \
    CONFIG.TID_WIDTH {0} \
    CONFIG.TUSER_WIDTH {0} \
  ] $axis_net_in
  set axis_net_out [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:axis_rtl:1.0 axis_net_out ]

  # Create ports
  set address_high [ create_bd_port -dir I -from 15 -to 0 -type data address_high ]
  set_property -dict [ list \
    CONFIG.LAYERED_METADATA {xilinx.com:interface:datatypes:1.0 {DATA {datatype {name {attribs {resolve_type immediate dependency {} format string minimum {} maximum {}} value {}} bitwidth {attribs {resolve_type immediate dependency {} format long minimum {} maximum {}} value 16} bitoffset {attribs {resolve_type immediate dependency {} format long minimum {} maximum {}} value 0} integer {signed {attribs {resolve_type immediate dependency {} format bool minimum {} maximum {}} value false}}}}}} \
  ] $address_high
  set address_low [ create_bd_port -dir I -from 15 -to 0 -type data address_low ]
  set_property -dict [ list \
    CONFIG.LAYERED_METADATA {xilinx.com:interface:datatypes:1.0 {DATA {datatype {name {attribs {resolve_type immediate dependency {} format string minimum {} maximum {}} value {}} bitwidth {attribs {resolve_type immediate dependency {} format long minimum {} maximum {}} value 16} bitoffset {attribs {resolve_type immediate dependency {} format long minimum {} maximum {}} value 0} integer {signed {attribs {resolve_type immediate dependency {} format bool minimum {} maximum {}} value false}}}}}} \
  ] $address_low
  set clk [ create_bd_port -dir I -type clk clk ]
  set reset_n [ create_bd_port -dir I -type rst reset_n ]

  # Create instance: am_rx_0, and set properties
  set am_rx_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:am_rx:1.0 am_rx_0 ]

  # Create instance: am_tx_0, and set properties
  set am_tx_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:am_tx:1.0 am_tx_0 ]

  # Create instance: axi_datamover_0, and set properties
  set axi_datamover_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_datamover:5.1 axi_datamover_0 ]
  set_property -dict [ list \
    CONFIG.c_m_axi_mm2s_data_width {64} \
    CONFIG.c_m_axi_mm2s_id_width {0} \
    CONFIG.c_m_axi_s2mm_id_width {0} \
    CONFIG.c_m_axis_mm2s_tdata_width {64} \
    CONFIG.c_mm2s_btt_used {15} \
    CONFIG.c_mm2s_burst_size {256} \
    CONFIG.c_s2mm_btt_used {15} \
    CONFIG.c_s2mm_burst_size {256} \
  ] $axi_datamover_0

  # Create instance: axi_interconnect_0, and set properties
  # set axi_interconnect_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_interconnect_0 ]
  # set_property -dict [ list \
  #   CONFIG.NUM_MI {1} \
  #   CONFIG.NUM_SI {2} \
  # ] $axi_interconnect_0

  # Create instance: axis_interconnect_0, and set properties
  set axis_interconnect_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_interconnect:2.1 axis_interconnect_0 ]
  set_property -dict [ list \
    CONFIG.ARB_ON_TLAST {1} \
    CONFIG.NUM_MI {1} \
    CONFIG.NUM_SI {2} \
  ] $axis_interconnect_0

  # Create instance: axis_interconnect_1, and set properties
  set axis_interconnect_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_interconnect:2.1 axis_interconnect_1 ]
  set_property -dict [ list \
    CONFIG.ARB_ON_TLAST {1} \
    CONFIG.NUM_MI {1} \
    CONFIG.NUM_SI {2} \
  ] $axis_interconnect_1

  # Create instance: axis_interconnect_2, and set properties
  set axis_interconnect_2 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_interconnect:2.1 axis_interconnect_2 ]
  set_property -dict [ list \
    CONFIG.ARB_ON_TLAST {1} \
    CONFIG.NUM_MI {1} \
    CONFIG.NUM_SI {2} \
  ] $axis_interconnect_2

  # Create instance: hold_buffer_0, and set properties
  set hold_buffer_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:hold_buffer:1.0 hold_buffer_0 ]

  # Create instance: hold_buffer_1, and set properties
  set hold_buffer_1 [ create_bd_cell -type ip -vlnv xilinx.com:hls:hold_buffer:1.0 hold_buffer_1 ]

  # Create instance: xlconcat_0, and set properties
  set xlconcat_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlconcat:2.1 xlconcat_0 ]
  set_property -dict [ list \
    CONFIG.IN0_WIDTH {64} \
    CONFIG.IN1_WIDTH {8} \
  ] $xlconcat_0

  # Create instance: xlconcat_1, and set properties
  set xlconcat_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlconcat:2.1 xlconcat_1 ]
  set_property -dict [ list \
    CONFIG.IN0_WIDTH {64} \
    CONFIG.IN1_WIDTH {8} \
  ] $xlconcat_1

  # Create instance: xlconstant_0, and set properties
  set xlconstant_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlconstant:1.1 xlconstant_0 ]
  set_property -dict [ list \
    CONFIG.CONST_VAL {0} \
    CONFIG.CONST_WIDTH {8} \
  ] $xlconstant_0

  # Create instance: xpams_rx_0, and set properties
  set xpams_rx_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:xpams_rx:1.0 xpams_rx_0 ]

  # Create instance: xpams_tx_0, and set properties
  set xpams_tx_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:xpams_tx:1.0 xpams_tx_0 ]

  # Create interface connections
  connect_bd_intf_net -intf_net am_rx_0_axis_handler [get_bd_intf_pins am_rx_0/axis_handler] [get_bd_intf_pins hold_buffer_0/axis_input]
  connect_bd_intf_net -intf_net am_rx_0_axis_s2mm [get_bd_intf_pins am_rx_0/axis_s2mm] [get_bd_intf_pins axi_datamover_0/S_AXIS_S2MM]
  connect_bd_intf_net -intf_net am_tx_0_axis_net [get_bd_intf_pins am_tx_0/axis_net] [get_bd_intf_pins hold_buffer_1/axis_input]
  connect_bd_intf_net -intf_net axi_datamover_0_M_AXIS_MM2S [get_bd_intf_pins am_tx_0/axis_mm2s] [get_bd_intf_pins axi_datamover_0/M_AXIS_MM2S]
  connect_bd_intf_net -intf_net axi_datamover_0_M_AXIS_MM2S_STS [get_bd_intf_pins am_tx_0/axis_mm2sStatus] [get_bd_intf_pins axi_datamover_0/M_AXIS_MM2S_STS]
  connect_bd_intf_net -intf_net axi_datamover_0_M_AXIS_S2MM_STS [get_bd_intf_pins am_rx_0/axis_s2mmStatus] [get_bd_intf_pins axi_datamover_0/M_AXIS_S2MM_STS]
  connect_bd_intf_net -intf_net axi_datamover_0_M_AXI_MM2S [get_bd_intf_pins axi_datamover_0/M_AXI_MM2S] [get_bd_intf_pins axi_interconnect_0/S00_AXI]
  connect_bd_intf_net -intf_net axi_datamover_0_M_AXI_S2MM [get_bd_intf_pins axi_datamover_0/M_AXI_S2MM] [get_bd_intf_pins axi_interconnect_0/S01_AXI]
  connect_bd_intf_net -intf_net axi_interconnect_0_M00_AXI [get_bd_intf_ports axi_mem] [get_bd_intf_pins axi_interconnect_0/M00_AXI]
  connect_bd_intf_net -intf_net axis_interconnect_0_M00_AXIS [get_bd_intf_pins am_tx_0/axis_kernel] [get_bd_intf_pins axis_interconnect_0/M00_AXIS]
  connect_bd_intf_net -intf_net axis_interconnect_1_M00_AXIS [get_bd_intf_ports axis_handler] [get_bd_intf_pins axis_interconnect_1/M00_AXIS]
  connect_bd_intf_net -intf_net axis_interconnect_2_M00_AXIS [get_bd_intf_ports axis_kernel_out] [get_bd_intf_pins axis_interconnect_2/M00_AXIS]
  connect_bd_intf_net -intf_net axis_kernel_in_1 [get_bd_intf_ports axis_kernel_in] [get_bd_intf_pins xpams_tx_0/axis_kernel_in]
  connect_bd_intf_net -intf_net axis_net_1 [get_bd_intf_ports axis_net_in] [get_bd_intf_pins am_rx_0/axis_net]
  connect_bd_intf_net -intf_net hold_buffer_0_axis_output [get_bd_intf_pins hold_buffer_0/axis_output] [get_bd_intf_pins xpams_rx_0/axis_rx]
  connect_bd_intf_net -intf_net hold_buffer_1_axis_output [get_bd_intf_ports axis_net_out] [get_bd_intf_pins hold_buffer_1/axis_output]
  connect_bd_intf_net -intf_net xpams_rx_0_axis_handler [get_bd_intf_pins axis_interconnect_1/S01_AXIS] [get_bd_intf_pins xpams_rx_0/axis_handler]
  connect_bd_intf_net -intf_net xpams_rx_0_axis_kernel_out [get_bd_intf_pins axis_interconnect_2/S01_AXIS] [get_bd_intf_pins xpams_rx_0/axis_kernel_out]
  connect_bd_intf_net -intf_net xpams_rx_0_axis_tx [get_bd_intf_pins axis_interconnect_0/S01_AXIS] [get_bd_intf_pins xpams_rx_0/axis_tx]
  connect_bd_intf_net -intf_net xpams_tx_0_axis_handler [get_bd_intf_pins axis_interconnect_1/S00_AXIS] [get_bd_intf_pins xpams_tx_0/axis_handler]
  connect_bd_intf_net -intf_net xpams_tx_0_axis_kernel_out [get_bd_intf_pins axis_interconnect_2/S00_AXIS] [get_bd_intf_pins xpams_tx_0/axis_kernel_out]
  connect_bd_intf_net -intf_net xpams_tx_0_axis_tx [get_bd_intf_pins axis_interconnect_0/S00_AXIS] [get_bd_intf_pins xpams_tx_0/axis_tx]

  # Create port connections
  connect_bd_net -net address_offset_high_V_1 [get_bd_ports address_high] [get_bd_pins xpams_tx_0/address_offset_high_V]
  connect_bd_net -net address_offset_low_V_1 [get_bd_ports address_low] [get_bd_pins xpams_tx_0/address_offset_low_V]
  connect_bd_net -net am_rx_0_axis_s2mmCommand_TDATA [get_bd_pins am_rx_0/axis_s2mmCommand_TDATA] [get_bd_pins xlconcat_0/In0]
  connect_bd_net -net am_rx_0_axis_s2mmCommand_TVALID [get_bd_pins am_rx_0/axis_s2mmCommand_TVALID] [get_bd_pins axi_datamover_0/s_axis_s2mm_cmd_tvalid]
  connect_bd_net -net am_rx_0_release_V [get_bd_pins am_rx_0/release_V] [get_bd_pins hold_buffer_0/dataRelease_V]
  connect_bd_net -net am_tx_0_axis_mm2sCommand_TDATA [get_bd_pins am_tx_0/axis_mm2sCommand_TDATA] [get_bd_pins xlconcat_1/In0]
  connect_bd_net -net am_tx_0_axis_mm2sCommand_TVALID [get_bd_pins am_tx_0/axis_mm2sCommand_TVALID] [get_bd_pins axi_datamover_0/s_axis_mm2s_cmd_tvalid]
  connect_bd_net -net am_tx_0_release_V [get_bd_pins am_tx_0/release_V] [get_bd_pins hold_buffer_1/dataRelease_V]
  connect_bd_net -net ap_clk_1 [get_bd_ports clk] [get_bd_pins am_rx_0/ap_clk] [get_bd_pins am_tx_0/ap_clk] [get_bd_pins axi_datamover_0/m_axi_mm2s_aclk] [get_bd_pins axi_datamover_0/m_axi_s2mm_aclk] [get_bd_pins axi_datamover_0/m_axis_mm2s_cmdsts_aclk] [get_bd_pins axi_datamover_0/m_axis_s2mm_cmdsts_awclk] [get_bd_pins axi_interconnect_0/ACLK] [get_bd_pins axi_interconnect_0/M00_ACLK] [get_bd_pins axi_interconnect_0/S00_ACLK] [get_bd_pins axi_interconnect_0/S01_ACLK] [get_bd_pins axis_interconnect_0/ACLK] [get_bd_pins axis_interconnect_0/M00_AXIS_ACLK] [get_bd_pins axis_interconnect_0/S00_AXIS_ACLK] [get_bd_pins axis_interconnect_0/S01_AXIS_ACLK] [get_bd_pins axis_interconnect_1/ACLK] [get_bd_pins axis_interconnect_1/M00_AXIS_ACLK] [get_bd_pins axis_interconnect_1/S00_AXIS_ACLK] [get_bd_pins axis_interconnect_1/S01_AXIS_ACLK] [get_bd_pins axis_interconnect_2/ACLK] [get_bd_pins axis_interconnect_2/M00_AXIS_ACLK] [get_bd_pins axis_interconnect_2/S00_AXIS_ACLK] [get_bd_pins axis_interconnect_2/S01_AXIS_ACLK] [get_bd_pins hold_buffer_0/ap_clk] [get_bd_pins hold_buffer_1/ap_clk] [get_bd_pins xpams_rx_0/ap_clk] [get_bd_pins xpams_tx_0/ap_clk]
  connect_bd_net -net ap_rst_n_1 [get_bd_ports reset_n] [get_bd_pins am_rx_0/ap_rst_n] [get_bd_pins am_tx_0/ap_rst_n] [get_bd_pins axi_datamover_0/m_axi_mm2s_aresetn] [get_bd_pins axi_datamover_0/m_axi_s2mm_aresetn] [get_bd_pins axi_datamover_0/m_axis_mm2s_cmdsts_aresetn] [get_bd_pins axi_datamover_0/m_axis_s2mm_cmdsts_aresetn] [get_bd_pins axi_interconnect_0/ARESETN] [get_bd_pins axi_interconnect_0/M00_ARESETN] [get_bd_pins axi_interconnect_0/S00_ARESETN] [get_bd_pins axi_interconnect_0/S01_ARESETN] [get_bd_pins axis_interconnect_0/ARESETN] [get_bd_pins axis_interconnect_0/M00_AXIS_ARESETN] [get_bd_pins axis_interconnect_0/S00_AXIS_ARESETN] [get_bd_pins axis_interconnect_0/S01_AXIS_ARESETN] [get_bd_pins axis_interconnect_1/ARESETN] [get_bd_pins axis_interconnect_1/M00_AXIS_ARESETN] [get_bd_pins axis_interconnect_1/S00_AXIS_ARESETN] [get_bd_pins axis_interconnect_1/S01_AXIS_ARESETN] [get_bd_pins axis_interconnect_2/ARESETN] [get_bd_pins axis_interconnect_2/M00_AXIS_ARESETN] [get_bd_pins axis_interconnect_2/S00_AXIS_ARESETN] [get_bd_pins axis_interconnect_2/S01_AXIS_ARESETN] [get_bd_pins hold_buffer_0/ap_rst_n] [get_bd_pins hold_buffer_1/ap_rst_n] [get_bd_pins xpams_rx_0/ap_rst_n] [get_bd_pins xpams_tx_0/ap_rst_n]
  connect_bd_net -net axi_datamover_0_s_axis_mm2s_cmd_tready [get_bd_pins am_tx_0/axis_mm2sCommand_TREADY] [get_bd_pins axi_datamover_0/s_axis_mm2s_cmd_tready]
  connect_bd_net -net axi_datamover_0_s_axis_s2mm_cmd_tready [get_bd_pins am_rx_0/axis_s2mmCommand_TREADY] [get_bd_pins axi_datamover_0/s_axis_s2mm_cmd_tready]
  connect_bd_net -net xlconcat_0_dout [get_bd_pins axi_datamover_0/s_axis_s2mm_cmd_tdata] [get_bd_pins xlconcat_0/dout]
  connect_bd_net -net xlconcat_1_dout [get_bd_pins axi_datamover_0/s_axis_mm2s_cmd_tdata] [get_bd_pins xlconcat_1/dout]
  connect_bd_net -net xlconstant_0_dout [get_bd_pins xlconcat_0/In1] [get_bd_pins xlconcat_1/In1] [get_bd_pins xlconstant_0/dout]

  # Create address segments
  create_bd_addr_seg -range 0x000100000000 -offset 0x00000000 [get_bd_addr_spaces axi_datamover_0/Data_MM2S] [get_bd_addr_segs axi_mem/Reg] SEG_axi_mem_Reg
  create_bd_addr_seg -range 0x000100000000 -offset 0x00000000 [get_bd_addr_spaces axi_datamover_0/Data_S2MM] [get_bd_addr_segs axi_mem/Reg] SEG_axi_mem_Reg


  # Restore current instance
  current_bd_instance $oldCurInst

  save_bd_design
  close_bd_design [get_bd_designs $bd_name]
}

}

################################################################
# Check if script is running in correct Vivado version.
################################################################
set current_vivado_version [version -short]

if { [string first 2017.2 $current_vivado_version] != -1 } {
  2017_2::create_GAScore GAScore
} else {
  puts ""
  catch {common::send_msg_id "BD_TCL-109" "ERROR" "Unsupported Vivado version:\
    $current_vivado_version for generating the GAScore"}

  return 1
}


