# Hierarchical cell: fpga_1
proc create_hier_cell_fpga_1 { parentCell nameHier } {

  variable script_folder

  if { $parentCell eq "" || $nameHier eq "" } {
     catch {common::send_msg_id "BD_TCL-102" "ERROR" "create_hier_cell_fpga_1() - Empty argument(s)!"}
     return
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_msg_id "BD_TCL-100" "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_msg_id "BD_TCL-101" "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj

  # Create cell and set as current instance
  set hier_obj [create_bd_cell -type hier $nameHier]
  current_bd_instance $hier_obj

  # Create interface pins
  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 axi_instr_1
  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:axis_rtl:1.0 net_in
  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:axis_rtl:1.0 net_out

  # Create pins
  create_bd_pin -dir I -type clk clock
  create_bd_pin -dir I -from 15 -to 0 -type data id
  create_bd_pin -dir I -type rst reset_n

  # Create instance: GAScore_0, and set properties
  set GAScore_0 [ create_bd_cell -type ip -vlnv varun.org:varun:GAScore:1.0 GAScore_0 ]
  set_property -dict [ list \
CONFIG.ADDRESS_HIGH {1} \
CONFIG.ADDRESS_LOW {1} \
 ] $GAScore_0

  # Create instance: axi_bram_ctrl_0, and set properties
  set axi_bram_ctrl_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_bram_ctrl:4.0 axi_bram_ctrl_0 ]
  set_property -dict [ list \
CONFIG.DATA_WIDTH {64} \
CONFIG.ECC_TYPE {0} \
 ] $axi_bram_ctrl_0

  # Create instance: axi_interconnect_0, and set properties
  set axi_interconnect_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_interconnect_0 ]
  set_property -dict [ list \
CONFIG.NUM_MI {1} \
CONFIG.NUM_SI {1} \
 ] $axi_interconnect_0

  # Create instance: axi_interconnect_1, and set properties
  set axi_interconnect_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_interconnect_1 ]
  set_property -dict [ list \
CONFIG.NUM_MI {1} \
CONFIG.NUM_SI {2} \
 ] $axi_interconnect_1

  # Create instance: axi_interconnect_2, and set properties
  set axi_interconnect_2 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_interconnect_2 ]
  set_property -dict [ list \
CONFIG.NUM_MI {1} \
CONFIG.NUM_SI {1} \
 ] $axi_interconnect_2

  # Create instance: axi_timer_0, and set properties
  set axi_timer_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_timer:2.0 axi_timer_0 ]

  # Create instance: blk_mem_gen_0, and set properties
  set blk_mem_gen_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:blk_mem_gen:8.3 blk_mem_gen_0 ]
  set_property -dict [ list \
CONFIG.Enable_32bit_Address {true} \
CONFIG.Enable_B {Use_ENB_Pin} \
CONFIG.Memory_Type {True_Dual_Port_RAM} \
CONFIG.Port_B_Clock {100} \
CONFIG.Port_B_Enable_Rate {100} \
CONFIG.Port_B_Write_Rate {50} \
CONFIG.Register_PortA_Output_of_Memory_Primitives {false} \
CONFIG.Use_RSTA_Pin {true} \
CONFIG.Use_RSTB_Pin {true} \
CONFIG.use_bram_block {BRAM_Controller} \
 ] $blk_mem_gen_0

  # Create instance: hls_kernel_0, and set properties
  set hls_kernel_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:hls_kernel:1.0 hls_kernel_0 ]

  # Create interface connections
  connect_bd_intf_net -intf_net GAScore_0_axi_local [get_bd_intf_pins GAScore_0/axi_local] [get_bd_intf_pins axi_interconnect_1/S00_AXI]
  connect_bd_intf_net -intf_net GAScore_0_kernel_out [get_bd_intf_pins GAScore_0/kernel_out] [get_bd_intf_pins hls_kernel_0/in_r]
  connect_bd_intf_net -intf_net GAScore_0_net_out [get_bd_intf_pins net_out] [get_bd_intf_pins GAScore_0/net_out]
  connect_bd_intf_net -intf_net S01_AXI_1 [get_bd_intf_pins GAScore_0/axi_remote] [get_bd_intf_pins axi_interconnect_1/S01_AXI]
  connect_bd_intf_net -intf_net axi_bram_ctrl_0_BRAM_PORTA [get_bd_intf_pins axi_bram_ctrl_0/BRAM_PORTA] [get_bd_intf_pins blk_mem_gen_0/BRAM_PORTA]
  connect_bd_intf_net -intf_net axi_bram_ctrl_0_BRAM_PORTB [get_bd_intf_pins axi_bram_ctrl_0/BRAM_PORTB] [get_bd_intf_pins blk_mem_gen_0/BRAM_PORTB]
  connect_bd_intf_net -intf_net axi_interconnect_0_M00_AXI [get_bd_intf_pins GAScore_0/ctrl_bus_0] [get_bd_intf_pins axi_interconnect_0/M00_AXI]
  connect_bd_intf_net -intf_net axi_interconnect_1_M00_AXI [get_bd_intf_pins axi_bram_ctrl_0/S_AXI] [get_bd_intf_pins axi_interconnect_1/M00_AXI]
  connect_bd_intf_net -intf_net axi_interconnect_2_M00_AXI [get_bd_intf_pins axi_interconnect_2/M00_AXI] [get_bd_intf_pins axi_timer_0/S_AXI]
  connect_bd_intf_net -intf_net hls_kernel_0_m_axi_axi_timer [get_bd_intf_pins axi_interconnect_2/S00_AXI] [get_bd_intf_pins hls_kernel_0/m_axi_axi_timer]
  connect_bd_intf_net -intf_net hls_kernel_0_m_axi_handler_ctrl [get_bd_intf_pins axi_interconnect_0/S00_AXI] [get_bd_intf_pins hls_kernel_0/m_axi_handler_ctrl]
  connect_bd_intf_net -intf_net hls_kernel_0_m_axi_instr_mem [get_bd_intf_pins axi_instr_1] [get_bd_intf_pins hls_kernel_0/m_axi_instr_mem]
  connect_bd_intf_net -intf_net hls_kernel_0_m_axi_local_mem [get_bd_intf_pins GAScore_0/kernel_mem_0] [get_bd_intf_pins hls_kernel_0/m_axi_local_mem]
  connect_bd_intf_net -intf_net hls_kernel_0_out_r [get_bd_intf_pins GAScore_0/kernel_in] [get_bd_intf_pins hls_kernel_0/out_r]
  connect_bd_intf_net -intf_net net_in_1 [get_bd_intf_pins net_in] [get_bd_intf_pins GAScore_0/net_in]

  # Create port connections
  connect_bd_net -net ap_clk_1 [get_bd_pins clock] [get_bd_pins GAScore_0/clock] [get_bd_pins axi_bram_ctrl_0/s_axi_aclk] [get_bd_pins axi_interconnect_0/ACLK] [get_bd_pins axi_interconnect_0/M00_ACLK] [get_bd_pins axi_interconnect_0/S00_ACLK] [get_bd_pins axi_interconnect_1/ACLK] [get_bd_pins axi_interconnect_1/M00_ACLK] [get_bd_pins axi_interconnect_1/S00_ACLK] [get_bd_pins axi_interconnect_1/S01_ACLK] [get_bd_pins axi_interconnect_2/ACLK] [get_bd_pins axi_interconnect_2/M00_ACLK] [get_bd_pins axi_interconnect_2/S00_ACLK] [get_bd_pins axi_timer_0/s_axi_aclk] [get_bd_pins hls_kernel_0/ap_clk]
  connect_bd_net -net ap_rst_n_1 [get_bd_pins reset_n] [get_bd_pins GAScore_0/reset_n] [get_bd_pins axi_bram_ctrl_0/s_axi_aresetn] [get_bd_pins axi_interconnect_0/ARESETN] [get_bd_pins axi_interconnect_0/M00_ARESETN] [get_bd_pins axi_interconnect_0/S00_ARESETN] [get_bd_pins axi_interconnect_1/ARESETN] [get_bd_pins axi_interconnect_1/M00_ARESETN] [get_bd_pins axi_interconnect_1/S00_ARESETN] [get_bd_pins axi_interconnect_1/S01_ARESETN] [get_bd_pins axi_interconnect_2/ARESETN] [get_bd_pins axi_interconnect_2/M00_ARESETN] [get_bd_pins axi_interconnect_2/S00_ARESETN] [get_bd_pins axi_timer_0/s_axi_aresetn] [get_bd_pins hls_kernel_0/ap_rst_n]
  connect_bd_net -net id_0_1 [get_bd_pins id] [get_bd_pins hls_kernel_0/id]

  # Restore current instance
  current_bd_instance $oldCurInst
}

# Hierarchical cell: fpga_0
proc create_hier_cell_fpga_0 { parentCell nameHier } {

  variable script_folder

  if { $parentCell eq "" || $nameHier eq "" } {
     catch {common::send_msg_id "BD_TCL-102" "ERROR" "create_hier_cell_fpga_0() - Empty argument(s)!"}
     return
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_msg_id "BD_TCL-100" "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_msg_id "BD_TCL-101" "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj

  # Create cell and set as current instance
  set hier_obj [create_bd_cell -type hier $nameHier]
  current_bd_instance $hier_obj

  # Create interface pins
  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 axi_instr_0
  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:axis_rtl:1.0 net_in
  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:axis_rtl:1.0 net_out

  # Create pins
  create_bd_pin -dir I -type clk clock
  create_bd_pin -dir I -from 15 -to 0 -type data id
  create_bd_pin -dir I -type rst reset_n

  # Create instance: GAScore_0, and set properties
  set GAScore_0 [ create_bd_cell -type ip -vlnv varun.org:varun:GAScore:1.0 GAScore_0 ]
  set_property -dict [ list \
CONFIG.ADDRESS_HIGH {0} \
 ] $GAScore_0

  # Create instance: axi_bram_ctrl_0, and set properties
  set axi_bram_ctrl_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_bram_ctrl:4.0 axi_bram_ctrl_0 ]
  set_property -dict [ list \
CONFIG.DATA_WIDTH {64} \
CONFIG.ECC_TYPE {0} \
 ] $axi_bram_ctrl_0

  # Create instance: axi_interconnect_0, and set properties
  set axi_interconnect_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_interconnect_0 ]
  set_property -dict [ list \
CONFIG.NUM_MI {1} \
CONFIG.NUM_SI {1} \
 ] $axi_interconnect_0

  # Create instance: axi_interconnect_1, and set properties
  set axi_interconnect_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_interconnect_1 ]
  set_property -dict [ list \
CONFIG.NUM_MI {1} \
CONFIG.NUM_SI {2} \
 ] $axi_interconnect_1

  # Create instance: axi_interconnect_2, and set properties
  set axi_interconnect_2 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_interconnect_2 ]
  set_property -dict [ list \
CONFIG.NUM_MI {1} \
CONFIG.NUM_SI {1} \
 ] $axi_interconnect_2

  # Create instance: axi_timer_0, and set properties
  set axi_timer_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_timer:2.0 axi_timer_0 ]

  # Create instance: blk_mem_gen_0, and set properties
  set blk_mem_gen_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:blk_mem_gen:8.3 blk_mem_gen_0 ]
  set_property -dict [ list \
CONFIG.Enable_32bit_Address {true} \
CONFIG.Enable_B {Use_ENB_Pin} \
CONFIG.Memory_Type {True_Dual_Port_RAM} \
CONFIG.Port_B_Clock {100} \
CONFIG.Port_B_Enable_Rate {100} \
CONFIG.Port_B_Write_Rate {50} \
CONFIG.Register_PortA_Output_of_Memory_Primitives {false} \
CONFIG.Use_RSTA_Pin {true} \
CONFIG.Use_RSTB_Pin {true} \
CONFIG.use_bram_block {BRAM_Controller} \
 ] $blk_mem_gen_0

  # Create instance: hls_kernel_0, and set properties
  set hls_kernel_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:hls_kernel:1.0 hls_kernel_0 ]

  # Create interface connections
  connect_bd_intf_net -intf_net GAScore_0_axi_local [get_bd_intf_pins GAScore_0/axi_local] [get_bd_intf_pins axi_interconnect_1/S00_AXI]
  connect_bd_intf_net -intf_net GAScore_0_kernel_out [get_bd_intf_pins GAScore_0/kernel_out] [get_bd_intf_pins hls_kernel_0/in_r]
  connect_bd_intf_net -intf_net GAScore_0_net_out [get_bd_intf_pins net_out] [get_bd_intf_pins GAScore_0/net_out]
  connect_bd_intf_net -intf_net S01_AXI_1 [get_bd_intf_pins GAScore_0/axi_remote] [get_bd_intf_pins axi_interconnect_1/S01_AXI]
  connect_bd_intf_net -intf_net axi_bram_ctrl_0_BRAM_PORTA [get_bd_intf_pins axi_bram_ctrl_0/BRAM_PORTA] [get_bd_intf_pins blk_mem_gen_0/BRAM_PORTA]
  connect_bd_intf_net -intf_net axi_bram_ctrl_0_BRAM_PORTB [get_bd_intf_pins axi_bram_ctrl_0/BRAM_PORTB] [get_bd_intf_pins blk_mem_gen_0/BRAM_PORTB]
  connect_bd_intf_net -intf_net axi_interconnect_0_M00_AXI [get_bd_intf_pins GAScore_0/ctrl_bus_0] [get_bd_intf_pins axi_interconnect_0/M00_AXI]
  connect_bd_intf_net -intf_net axi_interconnect_1_M00_AXI [get_bd_intf_pins axi_bram_ctrl_0/S_AXI] [get_bd_intf_pins axi_interconnect_1/M00_AXI]
  connect_bd_intf_net -intf_net axi_interconnect_2_M00_AXI [get_bd_intf_pins axi_interconnect_2/M00_AXI] [get_bd_intf_pins axi_timer_0/S_AXI]
  connect_bd_intf_net -intf_net hls_kernel_0_m_axi_axi_timer [get_bd_intf_pins axi_interconnect_2/S00_AXI] [get_bd_intf_pins hls_kernel_0/m_axi_axi_timer]
  connect_bd_intf_net -intf_net hls_kernel_0_m_axi_handler_ctrl [get_bd_intf_pins axi_interconnect_0/S00_AXI] [get_bd_intf_pins hls_kernel_0/m_axi_handler_ctrl]
  connect_bd_intf_net -intf_net hls_kernel_0_m_axi_instr_mem [get_bd_intf_pins axi_instr_0] [get_bd_intf_pins hls_kernel_0/m_axi_instr_mem]
  connect_bd_intf_net -intf_net hls_kernel_0_m_axi_local_mem [get_bd_intf_pins GAScore_0/kernel_mem_0] [get_bd_intf_pins hls_kernel_0/m_axi_local_mem]
  connect_bd_intf_net -intf_net hls_kernel_0_out_r [get_bd_intf_pins GAScore_0/kernel_in] [get_bd_intf_pins hls_kernel_0/out_r]
  connect_bd_intf_net -intf_net net_in_1 [get_bd_intf_pins net_in] [get_bd_intf_pins GAScore_0/net_in]

  # Create port connections
  connect_bd_net -net ap_clk_1 [get_bd_pins clock] [get_bd_pins GAScore_0/clock] [get_bd_pins axi_bram_ctrl_0/s_axi_aclk] [get_bd_pins axi_interconnect_0/ACLK] [get_bd_pins axi_interconnect_0/M00_ACLK] [get_bd_pins axi_interconnect_0/S00_ACLK] [get_bd_pins axi_interconnect_1/ACLK] [get_bd_pins axi_interconnect_1/M00_ACLK] [get_bd_pins axi_interconnect_1/S00_ACLK] [get_bd_pins axi_interconnect_1/S01_ACLK] [get_bd_pins axi_interconnect_2/ACLK] [get_bd_pins axi_interconnect_2/M00_ACLK] [get_bd_pins axi_interconnect_2/S00_ACLK] [get_bd_pins axi_timer_0/s_axi_aclk] [get_bd_pins hls_kernel_0/ap_clk]
  connect_bd_net -net ap_rst_n_1 [get_bd_pins reset_n] [get_bd_pins GAScore_0/reset_n] [get_bd_pins axi_bram_ctrl_0/s_axi_aresetn] [get_bd_pins axi_interconnect_0/ARESETN] [get_bd_pins axi_interconnect_0/M00_ARESETN] [get_bd_pins axi_interconnect_0/S00_ARESETN] [get_bd_pins axi_interconnect_1/ARESETN] [get_bd_pins axi_interconnect_1/M00_ARESETN] [get_bd_pins axi_interconnect_1/S00_ARESETN] [get_bd_pins axi_interconnect_1/S01_ARESETN] [get_bd_pins axi_interconnect_2/ARESETN] [get_bd_pins axi_interconnect_2/M00_ARESETN] [get_bd_pins axi_interconnect_2/S00_ARESETN] [get_bd_pins axi_timer_0/s_axi_aresetn] [get_bd_pins hls_kernel_0/ap_rst_n]
  connect_bd_net -net id_0_1 [get_bd_pins id] [get_bd_pins hls_kernel_0/id]

  # Restore current instance
  current_bd_instance $oldCurInst
}

# Procedure to create entire design; Provide argument to make
# procedure reusable. If parentCell is "", will use root.
proc create_bd { bd_name } {

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

  set_property ip_repo_paths ${::env(SHOAL_PATH)}/repo/${::env(SHOAL_VIVADO_VERSION)}/${::env(SHOAL_PART_FAMILY)} [current_project]
  update_ip_catalog -rebuild -scan_changes

  if { $parentCell eq "" } {
     set parentCell [get_bd_cells /]
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_msg_id "BD_TCL-100" "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_msg_id "BD_TCL-101" "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj


  # Create interface ports
  set axi_instr_0 [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 axi_instr_0 ]
  set_property -dict [ list \
CONFIG.ADDR_WIDTH {32} \
CONFIG.DATA_WIDTH {64} \
CONFIG.NUM_READ_OUTSTANDING {2} \
CONFIG.NUM_WRITE_OUTSTANDING {2} \
CONFIG.PROTOCOL {AXI4} \
 ] $axi_instr_0
  set axi_instr_1 [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 axi_instr_1 ]
  set_property -dict [ list \
CONFIG.ADDR_WIDTH {32} \
CONFIG.DATA_WIDTH {64} \
CONFIG.NUM_READ_OUTSTANDING {2} \
CONFIG.NUM_WRITE_OUTSTANDING {2} \
CONFIG.PROTOCOL {AXI4} \
 ] $axi_instr_1

  # Create ports
  set clock [ create_bd_port -dir I -type clk clock ]
  set id_0 [ create_bd_port -dir I -from 15 -to 0 id_0 ]
  set id_1 [ create_bd_port -dir I -from 15 -to 0 id_1 ]
  set reset_n [ create_bd_port -dir I -type rst reset_n ]

  # Create instance: fpga_0
  create_hier_cell_fpga_0 [current_bd_instance .] fpga_0

  # Create instance: fpga_1
  create_hier_cell_fpga_1 [current_bd_instance .] fpga_1

  # Create interface connections
  connect_bd_intf_net -intf_net fpga_0_axi_instr_0 [get_bd_intf_ports axi_instr_0] [get_bd_intf_pins fpga_0/axi_instr_0]
  connect_bd_intf_net -intf_net fpga_0_net_out [get_bd_intf_pins fpga_0/net_out] [get_bd_intf_pins fpga_1/net_in]
  connect_bd_intf_net -intf_net fpga_1_axi_local [get_bd_intf_ports axi_instr_1] [get_bd_intf_pins fpga_1/axi_instr_1]
  connect_bd_intf_net -intf_net net_in_1 [get_bd_intf_pins fpga_0/net_in] [get_bd_intf_pins fpga_1/net_out]

  # Create port connections
  connect_bd_net -net ap_clk_1 [get_bd_ports clock] [get_bd_pins fpga_0/clock] [get_bd_pins fpga_1/clock]
  connect_bd_net -net ap_rst_n_1 [get_bd_ports reset_n] [get_bd_pins fpga_0/reset_n] [get_bd_pins fpga_1/reset_n]
  connect_bd_net -net id_0_1 [get_bd_ports id_0] [get_bd_pins fpga_0/id]
  connect_bd_net -net id_1_1 [get_bd_ports id_1] [get_bd_pins fpga_1/id]

  # Create address segments
  create_bd_addr_seg -range 0x00001000 -offset 0x00000000 [get_bd_addr_spaces fpga_0/GAScore_0/axi_local] [get_bd_addr_segs fpga_0/axi_bram_ctrl_0/S_AXI/Mem0] SEG_axi_bram_ctrl_0_Mem0
  create_bd_addr_seg -range 0x00001000 -offset 0x00000000 [get_bd_addr_spaces fpga_0/GAScore_0/axi_remote] [get_bd_addr_segs fpga_0/axi_bram_ctrl_0/S_AXI/Mem0] SEG_axi_bram_ctrl_0_Mem0
  create_bd_addr_seg -range 0x00001000 -offset 0x00000000 [get_bd_addr_spaces fpga_0/hls_kernel_0/Data_m_axi_handler_ctrl] [get_bd_addr_segs fpga_0/GAScore_0/s_axi_ctrl_bus_00/reg0] SEG_GAScore_0_reg0
  create_bd_addr_seg -range 0x00001000 -offset 0x00000000 [get_bd_addr_spaces fpga_0/hls_kernel_0/Data_m_axi_local_mem] [get_bd_addr_segs fpga_0/GAScore_0/s_axi_00/reg0] SEG_GAScore_0_reg0
  create_bd_addr_seg -range 0x00001000 -offset 0x00000000 [get_bd_addr_spaces fpga_0/hls_kernel_0/Data_m_axi_instr_mem] [get_bd_addr_segs axi_instr_0/Reg] SEG_axi_instr_0_Reg
  create_bd_addr_seg -range 0x00001000 -offset 0x00000000 [get_bd_addr_spaces fpga_0/hls_kernel_0/Data_m_axi_axi_timer] [get_bd_addr_segs fpga_0/axi_timer_0/S_AXI/Reg] SEG_axi_timer_0_Reg
  create_bd_addr_seg -range 0x00001000 -offset 0x00000000 [get_bd_addr_spaces fpga_1/GAScore_0/axi_local] [get_bd_addr_segs fpga_1/axi_bram_ctrl_0/S_AXI/Mem0] SEG_axi_bram_ctrl_0_Mem0
  create_bd_addr_seg -range 0x00001000 -offset 0x00000000 [get_bd_addr_spaces fpga_1/GAScore_0/axi_remote] [get_bd_addr_segs fpga_1/axi_bram_ctrl_0/S_AXI/Mem0] SEG_axi_bram_ctrl_0_Mem0
  create_bd_addr_seg -range 0x00001000 -offset 0x00000000 [get_bd_addr_spaces fpga_1/hls_kernel_0/Data_m_axi_handler_ctrl] [get_bd_addr_segs fpga_1/GAScore_0/s_axi_ctrl_bus_00/reg0] SEG_GAScore_0_reg0
  create_bd_addr_seg -range 0x00001000 -offset 0x00000000 [get_bd_addr_spaces fpga_1/hls_kernel_0/Data_m_axi_local_mem] [get_bd_addr_segs fpga_1/GAScore_0/s_axi_00/reg0] SEG_GAScore_0_reg0
  create_bd_addr_seg -range 0x00001000 -offset 0x00000000 [get_bd_addr_spaces fpga_1/hls_kernel_0/Data_m_axi_instr_mem] [get_bd_addr_segs axi_instr_1/Reg] SEG_axi_instr_1_Reg
  create_bd_addr_seg -range 0x00001000 -offset 0x00000000 [get_bd_addr_spaces fpga_1/hls_kernel_0/Data_m_axi_axi_timer] [get_bd_addr_segs fpga_1/axi_timer_0/S_AXI/Reg] SEG_axi_timer_0_Reg


  # Restore current instance
  current_bd_instance $oldCurInst

  validate_bd_design
  save_bd_design
  close_bd_design [get_bd_designs $bd_name]
  make_wrapper -top -import -files [get_files $bd_name.bd]
}

proc create_memory {name coe_file} {

set ip_name [check_ip blk_mem_gen {8.3 8.4}]
if {$ip_name == -1} {
  return 1
}
create_ip -vlnv $ip_name -module_name $name
set_property -dict [list \
  CONFIG.Interface_Type {AXI4} \
  CONFIG.Write_Width_A {64} \
  CONFIG.Write_Depth_A {1024} \
  CONFIG.Load_Init_File {true} \
  CONFIG.Coe_File $coe_file \
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

# defines get_design_name
source ${::env(SHOAL_PATH)}/helper/utilities.tcl

set current_vivado_version [version -short]

create_bd hls_kernel_bd
create_memory blk_mem_gen_0 "${::env(SHOAL_PATH)}/tests/hls_kernel_0.coe"
create_memory blk_mem_gen_1 "${::env(SHOAL_PATH)}/tests/hls_kernel_1.coe"
