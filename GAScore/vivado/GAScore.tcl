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

  set_property ip_repo_paths ${::env(SHOAL_PATH)}/GAScore/repo [current_project]
  update_ip_catalog -rebuild -scan_changes

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
    CONFIG.DATA_WIDTH {64} \
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
    CONFIG.c_m_axi_mm2s_id_width {1} \
    CONFIG.c_m_axi_s2mm_awid {1} \
    CONFIG.c_m_axi_s2mm_data_width {64} \
    CONFIG.c_m_axi_s2mm_id_width {1} \
    CONFIG.c_m_axis_mm2s_tdata_width {64} \
    CONFIG.c_mm2s_btt_used {15} \
    CONFIG.c_mm2s_burst_size {256} \
    CONFIG.c_s2mm_btt_used {15} \
    CONFIG.c_s2mm_burst_size {256} \
  ] $axi_datamover_0

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
    CONFIG.M00_AXIS_HIGHTDEST {0xFFFFFFFF} \
  ] $axis_interconnect_2

  # Create instance: hold_buffer_0, and set properties
  set hold_buffer_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:hold_buffer:1.0 hold_buffer_0 ]

  # Create instance: hold_buffer_1, and set properties
  set hold_buffer_1 [ create_bd_cell -type ip -vlnv xilinx.com:hls:hold_buffer:1.0 hold_buffer_1 ]

  # Create instance: smartconnect_0, and set properties
  set smartconnect_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:smartconnect:1.0 smartconnect_0 ]
  set_property -dict [ list \
    CONFIG.ADVANCED_PROPERTIES { __view__ { functional { S00_Buffer { AR_SIZE 0 R_SIZE 0 } S01_Buffer { AW_SIZE 0 B_SIZE 0 W_SIZE 0 } M00_Buffer { AR_SIZE 0 AW_SIZE 0 B_SIZE 0 R_SIZE 0 W_SIZE 0 } } }} \
 ] $smartconnect_0

  # Create instance: xpams_rx_0, and set properties
  set xpams_rx_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:xpams_rx:1.0 xpams_rx_0 ]

  # Create instance: xpams_tx_0, and set properties
  set xpams_tx_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:xpams_tx:1.0 xpams_tx_0 ]

  # Create interface connections
  connect_bd_intf_net -intf_net am_rx_0_axis_xpams_rx [get_bd_intf_pins am_rx_0/axis_xpams_rx] [get_bd_intf_pins hold_buffer_0/axis_input]
  connect_bd_intf_net -intf_net am_rx_0_axis_s2mm [get_bd_intf_pins am_rx_0/axis_s2mm] [get_bd_intf_pins axi_datamover_0/S_AXIS_S2MM]
  connect_bd_intf_net -intf_net am_tx_0_axis_net [get_bd_intf_pins am_tx_0/axis_net] [get_bd_intf_pins hold_buffer_1/axis_input]
  connect_bd_intf_net -intf_net am_rx_0_axis_s2mmCommand_V_data_V [get_bd_intf_pins am_rx_0/axis_s2mmCommand_V_data_V] [get_bd_intf_pins axi_datamover_0/S_AXIS_S2MM_CMD]
  connect_bd_intf_net -intf_net am_tx_0_axis_mm2sCommand_V_data_V [get_bd_intf_pins am_tx_0/axis_mm2sCommand_V_data_V] [get_bd_intf_pins axi_datamover_0/S_AXIS_MM2S_CMD]
  connect_bd_intf_net -intf_net axi_datamover_0_M_AXIS_MM2S [get_bd_intf_pins am_tx_0/axis_mm2s] [get_bd_intf_pins axi_datamover_0/M_AXIS_MM2S]
  connect_bd_intf_net -intf_net axi_datamover_0_M_AXIS_MM2S_STS [get_bd_intf_pins am_tx_0/axis_mm2sStatus] [get_bd_intf_pins axi_datamover_0/M_AXIS_MM2S_STS]
  connect_bd_intf_net -intf_net axi_datamover_0_M_AXIS_S2MM_STS [get_bd_intf_pins am_rx_0/axis_s2mmStatus] [get_bd_intf_pins axi_datamover_0/M_AXIS_S2MM_STS]
  connect_bd_intf_net -intf_net axi_datamover_0_M_AXI_MM2S [get_bd_intf_pins axi_datamover_0/M_AXI_MM2S] [get_bd_intf_pins smartconnect_0/S00_AXI]
  connect_bd_intf_net -intf_net axi_datamover_0_M_AXI_S2MM [get_bd_intf_pins axi_datamover_0/M_AXI_S2MM] [get_bd_intf_pins smartconnect_0/S01_AXI]
  connect_bd_intf_net -intf_net smartconnect_0_M00_AXI [get_bd_intf_ports axi_mem] [get_bd_intf_pins smartconnect_0/M00_AXI]
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
  connect_bd_net -net am_rx_0_release_V [get_bd_pins am_rx_0/release_V] [get_bd_pins hold_buffer_0/dataRelease_V]
  connect_bd_net -net am_tx_0_release_V [get_bd_pins am_tx_0/release_V] [get_bd_pins hold_buffer_1/dataRelease_V]
  connect_bd_net -net ap_clk_1 [get_bd_ports clk] [get_bd_pins am_rx_0/ap_clk] [get_bd_pins am_tx_0/ap_clk] [get_bd_pins axi_datamover_0/m_axi_mm2s_aclk] [get_bd_pins axi_datamover_0/m_axi_s2mm_aclk] [get_bd_pins axi_datamover_0/m_axis_mm2s_cmdsts_aclk] [get_bd_pins axi_datamover_0/m_axis_s2mm_cmdsts_awclk] [get_bd_pins axis_interconnect_0/ACLK] [get_bd_pins axis_interconnect_0/M00_AXIS_ACLK] [get_bd_pins axis_interconnect_0/S00_AXIS_ACLK] [get_bd_pins axis_interconnect_0/S01_AXIS_ACLK] [get_bd_pins axis_interconnect_1/ACLK] [get_bd_pins axis_interconnect_1/M00_AXIS_ACLK] [get_bd_pins axis_interconnect_1/S00_AXIS_ACLK] [get_bd_pins axis_interconnect_1/S01_AXIS_ACLK] [get_bd_pins axis_interconnect_2/ACLK] [get_bd_pins axis_interconnect_2/M00_AXIS_ACLK] [get_bd_pins axis_interconnect_2/S00_AXIS_ACLK] [get_bd_pins axis_interconnect_2/S01_AXIS_ACLK] [get_bd_pins hold_buffer_0/ap_clk] [get_bd_pins hold_buffer_1/ap_clk] [get_bd_pins smartconnect_0/aclk] [get_bd_pins xpams_rx_0/ap_clk] [get_bd_pins xpams_tx_0/ap_clk]
  connect_bd_net -net ap_rst_n_1 [get_bd_ports reset_n] [get_bd_pins am_rx_0/ap_rst_n] [get_bd_pins am_tx_0/ap_rst_n] [get_bd_pins axi_datamover_0/m_axi_mm2s_aresetn] [get_bd_pins axi_datamover_0/m_axi_s2mm_aresetn] [get_bd_pins axi_datamover_0/m_axis_mm2s_cmdsts_aresetn] [get_bd_pins axi_datamover_0/m_axis_s2mm_cmdsts_aresetn] [get_bd_pins axis_interconnect_0/ARESETN] [get_bd_pins axis_interconnect_0/M00_AXIS_ARESETN] [get_bd_pins axis_interconnect_0/S00_AXIS_ARESETN] [get_bd_pins axis_interconnect_0/S01_AXIS_ARESETN] [get_bd_pins axis_interconnect_1/ARESETN] [get_bd_pins axis_interconnect_1/M00_AXIS_ARESETN] [get_bd_pins axis_interconnect_1/S00_AXIS_ARESETN] [get_bd_pins axis_interconnect_1/S01_AXIS_ARESETN] [get_bd_pins axis_interconnect_2/ARESETN] [get_bd_pins axis_interconnect_2/M00_AXIS_ARESETN] [get_bd_pins axis_interconnect_2/S00_AXIS_ARESETN] [get_bd_pins axis_interconnect_2/S01_AXIS_ARESETN] [get_bd_pins hold_buffer_0/ap_rst_n] [get_bd_pins hold_buffer_1/ap_rst_n] [get_bd_pins smartconnect_0/aresetn] [get_bd_pins xpams_rx_0/ap_rst_n] [get_bd_pins xpams_tx_0/ap_rst_n]

  # Create address segments
  create_bd_addr_seg -range 0x000100000000 -offset 0x00000000 [get_bd_addr_spaces axi_datamover_0/Data_MM2S] [get_bd_addr_segs axi_mem/Reg] SEG_axi_mem_Reg
  create_bd_addr_seg -range 0x000100000000 -offset 0x00000000 [get_bd_addr_spaces axi_datamover_0/Data_S2MM] [get_bd_addr_segs axi_mem/Reg] SEG_axi_mem_Reg


  # Restore current instance
  current_bd_instance $oldCurInst

  validate_bd_design
  save_bd_design
  close_bd_design [get_bd_designs $bd_name]
  make_wrapper -top -import -files [get_files $bd_name.bd]
}

proc create_memory {} {

create_ip -name blk_mem_gen -vendor xilinx.com -library ip -version 8.3 -module_name blk_mem_gen_0
set_property -dict [list \
  CONFIG.Interface_Type {AXI4} \
  CONFIG.Write_Width_A {64} \
  CONFIG.Write_Depth_A {1024} \
  CONFIG.Load_Init_File {true} \
  CONFIG.Coe_File {/media/sharm294/HDD_1TB/Home/Documents/masters/git_repos/shoal/GAScore/include/GAScore.coe} \
  CONFIG.Fill_Remaining_Memory_Locations {true} \
  CONFIG.Use_AXI_ID {true} \
  CONFIG.Memory_Type {Simple_Dual_Port_RAM} \
  CONFIG.Use_Byte_Write_Enable {true} \
  CONFIG.Byte_Size {8} \
  CONFIG.Assume_Synchronous_Clk {true} \
  CONFIG.Read_Width_A {64} \
  CONFIG.Operating_Mode_A {READ_FIRST} \
  CONFIG.Write_Width_B {64} \
  CONFIG.Read_Width_B {64} \
  CONFIG.Operating_Mode_B {READ_FIRST} \
  CONFIG.Enable_B {Use_ENB_Pin} \
  CONFIG.Register_PortA_Output_of_Memory_Primitives {false} \
  CONFIG.Use_RSTB_Pin {true} \
  CONFIG.Reset_Type {ASYNC} \
  CONFIG.Port_B_Clock {100} \
  CONFIG.Port_B_Enable_Rate {100} \
] [get_ips blk_mem_gen_0]

}

}

# defines get_design_name
source ${::env(SHOAL_PATH)}/share/utilities.tcl

set current_vivado_version [version -short]

if { [string first 2017.2 $current_vivado_version] != -1 } {
  2017_2::create_GAScore GAScore_bd
  2017_2::create_memory
} else {
  puts ""
  catch {common::send_msg_id "BD_TCL-109" "ERROR" "Unsupported Vivado version:\
    $current_vivado_version for generating the GAScore"}

  return 1
}
