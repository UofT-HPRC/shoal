proc create_GAScore {bd_name} {

  set design_name $bd_name
  
  set result [get_design_name $design_name]

  set cur_design [lindex $result 0]
  set design_name [lindex $result 1]
  set errMsg [lindex $result 2]
  set nRet [lindex $result 3]

  set_property ip_repo_paths ${::env(SHOAL_PATH)}/repo/${::env(SHOAL_VIVADO_VERSION)}/${::env(SHOAL_PART_FAMILY)} [current_project]
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
  set axi_local [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 axi_local ]
  set_property -dict [ list \
CONFIG.ADDR_WIDTH {32} \
CONFIG.DATA_WIDTH {64} \
CONFIG.NUM_READ_OUTSTANDING {2} \
CONFIG.NUM_WRITE_OUTSTANDING {2} \
CONFIG.PROTOCOL {AXI4} \
 ] $axi_local
  set axi_remote [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 axi_remote ]
  set_property -dict [ list \
CONFIG.ADDR_WIDTH {32} \
CONFIG.DATA_WIDTH {64} \
CONFIG.HAS_REGION {0} \
CONFIG.NUM_READ_OUTSTANDING {2} \
CONFIG.NUM_WRITE_OUTSTANDING {2} \
CONFIG.PROTOCOL {AXI4} \
 ] $axi_remote
  set ctrl_bus_0 [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 ctrl_bus_0 ]
  set_property -dict [ list \
CONFIG.ADDR_WIDTH {12} \
CONFIG.ARUSER_WIDTH {0} \
CONFIG.AWUSER_WIDTH {0} \
CONFIG.BUSER_WIDTH {0} \
CONFIG.DATA_WIDTH {32} \
CONFIG.HAS_BRESP {1} \
CONFIG.HAS_BURST {0} \
CONFIG.HAS_CACHE {0} \
CONFIG.HAS_LOCK {0} \
CONFIG.HAS_PROT {0} \
CONFIG.HAS_QOS {0} \
CONFIG.HAS_REGION {0} \
CONFIG.HAS_RRESP {1} \
CONFIG.HAS_WSTRB {1} \
CONFIG.ID_WIDTH {0} \
CONFIG.MAX_BURST_LENGTH {1} \
CONFIG.NUM_READ_OUTSTANDING {1} \
CONFIG.NUM_READ_THREADS {1} \
CONFIG.NUM_WRITE_OUTSTANDING {1} \
CONFIG.NUM_WRITE_THREADS {1} \
CONFIG.PROTOCOL {AXI4LITE} \
CONFIG.READ_WRITE_MODE {READ_WRITE} \
CONFIG.RUSER_BITS_PER_BYTE {0} \
CONFIG.RUSER_WIDTH {0} \
CONFIG.SUPPORTS_NARROW_BURST {0} \
CONFIG.WUSER_BITS_PER_BYTE {0} \
CONFIG.WUSER_WIDTH {0} \
 ] $ctrl_bus_0
  set ctrl_bus_1 [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 ctrl_bus_1 ]
  set_property -dict [ list \
CONFIG.ADDR_WIDTH {12} \
CONFIG.ARUSER_WIDTH {0} \
CONFIG.AWUSER_WIDTH {0} \
CONFIG.BUSER_WIDTH {0} \
CONFIG.DATA_WIDTH {32} \
CONFIG.HAS_BRESP {1} \
CONFIG.HAS_BURST {0} \
CONFIG.HAS_CACHE {0} \
CONFIG.HAS_LOCK {0} \
CONFIG.HAS_PROT {0} \
CONFIG.HAS_QOS {0} \
CONFIG.HAS_REGION {0} \
CONFIG.HAS_RRESP {1} \
CONFIG.HAS_WSTRB {1} \
CONFIG.ID_WIDTH {0} \
CONFIG.MAX_BURST_LENGTH {1} \
CONFIG.NUM_READ_OUTSTANDING {1} \
CONFIG.NUM_READ_THREADS {1} \
CONFIG.NUM_WRITE_OUTSTANDING {1} \
CONFIG.NUM_WRITE_THREADS {1} \
CONFIG.PROTOCOL {AXI4LITE} \
CONFIG.READ_WRITE_MODE {READ_WRITE} \
CONFIG.RUSER_BITS_PER_BYTE {0} \
CONFIG.RUSER_WIDTH {0} \
CONFIG.SUPPORTS_NARROW_BURST {0} \
CONFIG.WUSER_BITS_PER_BYTE {0} \
CONFIG.WUSER_WIDTH {0} \
 ] $ctrl_bus_1
  set kernel_in [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:axis_rtl:1.0 kernel_in ]
  set_property -dict [ list \
CONFIG.HAS_TKEEP {1} \
CONFIG.HAS_TLAST {1} \
CONFIG.HAS_TREADY {1} \
CONFIG.HAS_TSTRB {0} \
CONFIG.LAYERED_METADATA {undef} \
CONFIG.TDATA_NUM_BYTES {8} \
CONFIG.TDEST_WIDTH {0} \
CONFIG.TID_WIDTH {0} \
CONFIG.TUSER_WIDTH {0} \
 ] $kernel_in
  set kernel_mem_0 [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 kernel_mem_0 ]
  set_property -dict [ list \
CONFIG.ADDR_WIDTH {32} \
CONFIG.ARUSER_WIDTH {0} \
CONFIG.AWUSER_WIDTH {0} \
CONFIG.BUSER_WIDTH {0} \
CONFIG.DATA_WIDTH {64} \
CONFIG.HAS_BRESP {1} \
CONFIG.HAS_BURST {1} \
CONFIG.HAS_CACHE {1} \
CONFIG.HAS_LOCK {1} \
CONFIG.HAS_PROT {1} \
CONFIG.HAS_QOS {1} \
CONFIG.HAS_REGION {0} \
CONFIG.HAS_RRESP {1} \
CONFIG.HAS_WSTRB {1} \
CONFIG.ID_WIDTH {1} \
CONFIG.MAX_BURST_LENGTH {256} \
CONFIG.NUM_READ_OUTSTANDING {2} \
CONFIG.NUM_READ_THREADS {1} \
CONFIG.NUM_WRITE_OUTSTANDING {2} \
CONFIG.NUM_WRITE_THREADS {1} \
CONFIG.PROTOCOL {AXI4} \
CONFIG.READ_WRITE_MODE {READ_WRITE} \
CONFIG.RUSER_BITS_PER_BYTE {0} \
CONFIG.RUSER_WIDTH {0} \
CONFIG.SUPPORTS_NARROW_BURST {1} \
CONFIG.WUSER_BITS_PER_BYTE {0} \
CONFIG.WUSER_WIDTH {0} \
 ] $kernel_mem_0
  set kernel_mem_1 [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 kernel_mem_1 ]
  set_property -dict [ list \
CONFIG.ADDR_WIDTH {32} \
CONFIG.ARUSER_WIDTH {0} \
CONFIG.AWUSER_WIDTH {0} \
CONFIG.BUSER_WIDTH {0} \
CONFIG.DATA_WIDTH {64} \
CONFIG.HAS_BRESP {1} \
CONFIG.HAS_BURST {1} \
CONFIG.HAS_CACHE {1} \
CONFIG.HAS_LOCK {1} \
CONFIG.HAS_PROT {1} \
CONFIG.HAS_QOS {1} \
CONFIG.HAS_REGION {0} \
CONFIG.HAS_RRESP {1} \
CONFIG.HAS_WSTRB {1} \
CONFIG.ID_WIDTH {1} \
CONFIG.MAX_BURST_LENGTH {256} \
CONFIG.NUM_READ_OUTSTANDING {2} \
CONFIG.NUM_READ_THREADS {1} \
CONFIG.NUM_WRITE_OUTSTANDING {2} \
CONFIG.NUM_WRITE_THREADS {1} \
CONFIG.PROTOCOL {AXI4} \
CONFIG.READ_WRITE_MODE {READ_WRITE} \
CONFIG.RUSER_BITS_PER_BYTE {0} \
CONFIG.RUSER_WIDTH {0} \
CONFIG.SUPPORTS_NARROW_BURST {1} \
CONFIG.WUSER_BITS_PER_BYTE {0} \
CONFIG.WUSER_WIDTH {0} \
 ] $kernel_mem_1
  set kernel_out [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:axis_rtl:1.0 kernel_out ]
  set net_in [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:axis_rtl:1.0 net_in ]
  set_property -dict [ list \
CONFIG.HAS_TKEEP {1} \
CONFIG.HAS_TLAST {1} \
CONFIG.HAS_TREADY {1} \
CONFIG.HAS_TSTRB {0} \
CONFIG.LAYERED_METADATA {undef} \
CONFIG.TDATA_NUM_BYTES {8} \
CONFIG.TDEST_WIDTH {0} \
CONFIG.TID_WIDTH {0} \
CONFIG.TUSER_WIDTH {0} \
 ] $net_in
  set net_out [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:axis_rtl:1.0 net_out ]

  # Create ports
  set clock [ create_bd_port -dir I -type clk clock ]
  set interrupt_0 [ create_bd_port -dir O interrupt_0 ]
  set interrupt_1 [ create_bd_port -dir O interrupt_1 ]
  set reset_n [ create_bd_port -dir I -type rst reset_n ]

  # Create instance: GAScore_0, and set properties
  set GAScore_0 [ create_bd_cell -type ip -vlnv varun.org:varun:GAScore:1.0 GAScore_0 ]
  set_property -dict [ list \
CONFIG.NUM_KERNELS {2} \
 ] $GAScore_0

  # Create interface connections
  connect_bd_intf_net -intf_net GAScore_0_axi_local [get_bd_intf_ports axi_local] [get_bd_intf_pins GAScore_0/axi_local]
  connect_bd_intf_net -intf_net GAScore_0_axi_remote [get_bd_intf_ports axi_remote] [get_bd_intf_pins GAScore_0/axi_remote]
  connect_bd_intf_net -intf_net GAScore_0_kernel_out [get_bd_intf_ports kernel_out] [get_bd_intf_pins GAScore_0/kernel_out]
  connect_bd_intf_net -intf_net GAScore_0_net_out [get_bd_intf_ports net_out] [get_bd_intf_pins GAScore_0/net_out]
  connect_bd_intf_net -intf_net ctrl_bus_0_1 [get_bd_intf_ports ctrl_bus_0] [get_bd_intf_pins GAScore_0/ctrl_bus_0]
  connect_bd_intf_net -intf_net ctrl_bus_1_1 [get_bd_intf_ports ctrl_bus_1] [get_bd_intf_pins GAScore_0/ctrl_bus_1]
  connect_bd_intf_net -intf_net kernel_in_1 [get_bd_intf_ports kernel_in] [get_bd_intf_pins GAScore_0/kernel_in]
  connect_bd_intf_net -intf_net kernel_mem_0_1 [get_bd_intf_ports kernel_mem_0] [get_bd_intf_pins GAScore_0/kernel_mem_0]
  connect_bd_intf_net -intf_net kernel_mem_1_1 [get_bd_intf_ports kernel_mem_1] [get_bd_intf_pins GAScore_0/kernel_mem_1]
  connect_bd_intf_net -intf_net net_in_1 [get_bd_intf_ports net_in] [get_bd_intf_pins GAScore_0/net_in]

  # Create port connections
  connect_bd_net -net GAScore_0_interrupt_0 [get_bd_ports interrupt_0] [get_bd_pins GAScore_0/interrupt_0]
  connect_bd_net -net GAScore_0_interrupt_1 [get_bd_ports interrupt_1] [get_bd_pins GAScore_0/interrupt_1]
  connect_bd_net -net clock_1 [get_bd_ports clock] [get_bd_pins GAScore_0/clock]
  connect_bd_net -net reset_n_1 [get_bd_ports reset_n] [get_bd_pins GAScore_0/reset_n]

  # Create address segments
  create_bd_addr_seg -range 0x00010000 -offset 0x44A00000 [get_bd_addr_spaces GAScore_0/axi_local] [get_bd_addr_segs axi_local/Reg] SEG_axi_local_Reg
  create_bd_addr_seg -range 0x00010000 -offset 0x44A00000 [get_bd_addr_spaces GAScore_0/axi_remote] [get_bd_addr_segs axi_remote/Reg] SEG_axi_remote_Reg
  create_bd_addr_seg -range 0x00001000 -offset 0x00000000 [get_bd_addr_spaces ctrl_bus_0] [get_bd_addr_segs GAScore_0/s_axi_ctrl_bus_00/reg0] SEG_GAScore_0_reg0
  create_bd_addr_seg -range 0x00001000 -offset 0x00000000 [get_bd_addr_spaces ctrl_bus_1] [get_bd_addr_segs GAScore_0/s_axi_ctrl_bus_01/reg0] SEG_GAScore_0_reg0
  create_bd_addr_seg -range 0x000100000000 -offset 0x00000000 [get_bd_addr_spaces kernel_mem_0] [get_bd_addr_segs GAScore_0/s_axi_00/reg0] SEG_GAScore_0_reg0
  create_bd_addr_seg -range 0x000100000000 -offset 0x00000000 [get_bd_addr_spaces kernel_mem_1] [get_bd_addr_segs GAScore_0/s_axi_01/reg0] SEG_GAScore_0_reg0


  # Restore current instance
  current_bd_instance $oldCurInst

  save_bd_design
}


# defines get_design_name
source ${::env(SHOAL_PATH)}/helper/utilities.tcl

set current_vivado_version [version -short]

create_GAScore GAScore_benchmark

# needed for a weird simulation bug where Vivado can't find gascore
generate_target all [get_files GAScore_benchmark.bd]
