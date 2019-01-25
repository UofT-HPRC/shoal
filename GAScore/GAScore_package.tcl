set part_name ${::env(SHOAL_PART)}
set GAScore_path ${::env(SHOAL_PATH)}/GAScore
set project_path $GAScore_path/vivado/projects/${::env(SHOAL_VIVADO_VERSION)}/$part_name

open_project -part $part_name $project_path/GAScore/GAScore.xpr
set_property  ip_repo_paths  ${::env(SHOAL_PATH)}/GAScore/repo/${::env(SHOAL_VIVADO_VERSION)}/${::env(SHOAL_PART_FAMILY)} [current_project]
update_ip_catalog
update_files -from_files $GAScore_path/src/GAScore.v -to_files $GAScore_path/vivado/src/GAScore/GAScore.sv -filesets [get_filesets *]

generate_target all [get_files  $project_path/GAScore/GAScore.srcs/sources_1/bd/GAScore_bd/GAScore_bd.bd]
catch { config_ip_cache -export [get_ips -all GAScore_bd_am_rx_0_0] }
catch { config_ip_cache -export [get_ips -all GAScore_bd_am_tx_0_0] }
catch { config_ip_cache -export [get_ips -all GAScore_bd_axi_datamover_0_0] }
catch { config_ip_cache -export [get_ips -all GAScore_bd_hold_buffer_0_0] }
catch { config_ip_cache -export [get_ips -all GAScore_bd_hold_buffer_1_0] }
catch { config_ip_cache -export [get_ips -all GAScore_bd_smartconnect_0_0] }
catch { config_ip_cache -export [get_ips -all GAScore_bd_xpams_rx_0_0] }
catch { config_ip_cache -export [get_ips -all GAScore_bd_xpams_tx_0_0] }
catch { config_ip_cache -export [get_ips -all GAScore_bd_xbar_2] }
catch { config_ip_cache -export [get_ips -all GAScore_bd_xbar_1] }
catch { config_ip_cache -export [get_ips -all GAScore_bd_xbar_0] }
catch { config_ip_cache -export [get_ips -all GAScore_bd_s_arb_req_suppress_concat_2] }
catch { config_ip_cache -export [get_ips -all GAScore_bd_s01_data_fifo_2] }
catch { config_ip_cache -export [get_ips -all GAScore_bd_s00_data_fifo_2] }
catch { config_ip_cache -export [get_ips -all GAScore_bd_s_arb_req_suppress_concat_1] }
catch { config_ip_cache -export [get_ips -all GAScore_bd_s01_data_fifo_1] }
catch { config_ip_cache -export [get_ips -all GAScore_bd_s00_data_fifo_1] }
catch { config_ip_cache -export [get_ips -all GAScore_bd_s_arb_req_suppress_concat_0] }
catch { config_ip_cache -export [get_ips -all GAScore_bd_s01_data_fifo_0] }
catch { config_ip_cache -export [get_ips -all GAScore_bd_s00_data_fifo_0] }
export_ip_user_files -of_objects [get_files $project_path/GAScore/GAScore.srcs/sources_1/bd/GAScore_bd/GAScore_bd.bd] -no_script -sync -force -quiet
create_ip_run [get_files -of_objects [get_fileset sources_1] $project_path/GAScore/GAScore.srcs/sources_1/bd/GAScore_bd/GAScore_bd.bd]
launch_runs -jobs 2 {GAScore_bd_am_rx_0_0_synth_1 GAScore_bd_am_tx_0_0_synth_1 GAScore_bd_axi_datamover_0_0_synth_1 GAScore_bd_hold_buffer_0_0_synth_1 GAScore_bd_hold_buffer_1_0_synth_1 GAScore_bd_smartconnect_0_0_synth_1 GAScore_bd_xpams_rx_0_0_synth_1 GAScore_bd_xpams_tx_0_0_synth_1 GAScore_bd_xbar_2_synth_1 GAScore_bd_xbar_1_synth_1 GAScore_bd_xbar_0_synth_1 GAScore_bd_s_arb_req_suppress_concat_2_synth_1 GAScore_bd_s01_data_fifo_2_synth_1 GAScore_bd_s00_data_fifo_2_synth_1 GAScore_bd_s_arb_req_suppress_concat_1_synth_1 GAScore_bd_s01_data_fifo_1_synth_1 GAScore_bd_s00_data_fifo_1_synth_1 GAScore_bd_s_arb_req_suppress_concat_0_synth_1 GAScore_bd_s01_data_fifo_0_synth_1 GAScore_bd_s00_data_fifo_0_synth_1}
export_simulation -of_objects [get_files $project_path/GAScore/GAScore.srcs/sources_1/bd/GAScore_bd/GAScore_bd.bd] -directory $project_path/GAScore/GAScore.ip_user_files/sim_scripts -ip_user_files_dir $project_path/GAScore/GAScore.ip_user_files -ipstatic_source_dir $project_path/GAScore/GAScore.ip_user_files/ipstatic -lib_map_path [list {modelsim=$project_path/GAScore/GAScore.cache/compile_simlib/modelsim} {questa=$project_path/GAScore/GAScore.cache/compile_simlib/questa} {ies=$project_path/GAScore/GAScore.cache/compile_simlib/ies} {vcs=$project_path/GAScore/GAScore.cache/compile_simlib/vcs} {riviera=$project_path/GAScore/GAScore.cache/compile_simlib/riviera}] -use_ip_compiled_libs -force -quiet

ipx::package_project -root_dir $project_path/temp -vendor varun.org -library varun -taxonomy /UserIP -generated_files -import_files -set_current false -force
ipx::unload_core $project_path/temp/component.xml
ipx::edit_ip_in_project -upgrade true -name tmp_edit_project -directory $project_path/temp $project_path/temp/component.xml
update_compile_order -fileset sources_1
ipx::remove_file src/GAScore.coe [ipx::get_file_groups xilinx_anylanguagesynthesis -of_objects [ipx::current_core]]
ipx::remove_file src/GAScore_sv.dat [ipx::get_file_groups xilinx_anylanguagesynthesis -of_objects [ipx::current_core]]
ipx::remove_file src/GAScore.coe [ipx::get_file_groups xilinx_anylanguagebehavioralsimulation -of_objects [ipx::current_core]]
ipx::remove_file src/GAScore_sv.dat [ipx::get_file_groups xilinx_anylanguagebehavioralsimulation -of_objects [ipx::current_core]]

set_property tooltip {The lowest TDEST address that is addressable locally} [ipgui::get_guiparamspec -name "ADDRESS_LOW" -component [ipx::current_core] ]
set_property widget {textEdit} [ipgui::get_guiparamspec -name "ADDRESS_LOW" -component [ipx::current_core] ]
set_property tooltip {The highest TDEST address that is addressable locally} [ipgui::get_guiparamspec -name "ADDRESS_HIGH" -component [ipx::current_core] ]
set_property widget {textEdit} [ipgui::get_guiparamspec -name "ADDRESS_HIGH" -component [ipx::current_core] ]
set_property tooltip {Number of active kernels on this FPGA} [ipgui::get_guiparamspec -name "NUM_KERNELS" -component [ipx::current_core] ]
set_property widget {textEdit} [ipgui::get_guiparamspec -name "NUM_KERNELS" -component [ipx::current_core] ]
set_property value 1 [ipx::get_user_parameters NUM_KERNELS -of_objects [ipx::current_core]]
set_property value 1 [ipx::get_hdl_parameters NUM_KERNELS -of_objects [ipx::current_core]]
set_property value_validation_type range_long [ipx::get_user_parameters NUM_KERNELS -of_objects [ipx::current_core]]
set_property value_validation_range_minimum 1 [ipx::get_user_parameters NUM_KERNELS -of_objects [ipx::current_core]]
set_property value_validation_range_maximum 16 [ipx::get_user_parameters NUM_KERNELS -of_objects [ipx::current_core]]
set_property tooltip {Width (bits) of the addresses of AXI-Lite interfaces} [ipgui::get_guiparamspec -name "ADDR_WIDTH" -component [ipx::current_core] ]
set_property widget {textEdit} [ipgui::get_guiparamspec -name "ADDR_WIDTH" -component [ipx::current_core] ]
set_property value 12 [ipx::get_user_parameters ADDR_WIDTH -of_objects [ipx::current_core]]
set_property value 12 [ipx::get_hdl_parameters ADDR_WIDTH -of_objects [ipx::current_core]]
set_property value_validation_type range_long [ipx::get_user_parameters ADDR_WIDTH -of_objects [ipx::current_core]]
set_property value_validation_range_minimum 12 [ipx::get_user_parameters ADDR_WIDTH -of_objects [ipx::current_core]]
set_property value_validation_range_maximum 64 [ipx::get_user_parameters ADDR_WIDTH -of_objects [ipx::current_core]]
set_property tooltip {Width (bits) of the data of AXI-Lite interfaces} [ipgui::get_guiparamspec -name "DATA_WIDTH" -component [ipx::current_core] ]
set_property widget {comboBox} [ipgui::get_guiparamspec -name "DATA_WIDTH" -component [ipx::current_core] ]
set_property value_validation_type list [ipx::get_user_parameters DATA_WIDTH -of_objects [ipx::current_core]]
set_property value_validation_list {8 16 24 32 40 48 56 64 128} [ipx::get_user_parameters DATA_WIDTH -of_objects [ipx::current_core]]
set_property enablement_value false [ipx::get_user_parameters STRB_WIDTH -of_objects [ipx::current_core]]
ipgui::remove_param -component [ipx::current_core] [ipgui::get_guiparamspec -name "STRB_WIDTH" -component [ipx::current_core]]
set_property value_tcl_expr {$DATA_WIDTH / 8} [ipx::get_user_parameters STRB_WIDTH -of_objects [ipx::current_core]]

set_property name kernel_in [ipx::get_bus_interfaces axis_kernel_in -of_objects [ipx::current_core]]
set_property name kernel_out [ipx::get_bus_interfaces axis_kernel_out -of_objects [ipx::current_core]]
set_property name net_in [ipx::get_bus_interfaces axis_net_in -of_objects [ipx::current_core]]
set_property name net_out [ipx::get_bus_interfaces axis_net_out -of_objects [ipx::current_core]]

set_property name ctrl_bus_0 [ipx::get_bus_interfaces s_axi_ctrl_bus_00 -of_objects [ipx::current_core]]
set_property name ctrl_bus_1 [ipx::get_bus_interfaces s_axi_ctrl_bus_01 -of_objects [ipx::current_core]]
set_property name ctrl_bus_2 [ipx::get_bus_interfaces s_axi_ctrl_bus_02 -of_objects [ipx::current_core]]
set_property name ctrl_bus_3 [ipx::get_bus_interfaces s_axi_ctrl_bus_03 -of_objects [ipx::current_core]]
set_property name ctrl_bus_4 [ipx::get_bus_interfaces s_axi_ctrl_bus_04 -of_objects [ipx::current_core]]
set_property name ctrl_bus_5 [ipx::get_bus_interfaces s_axi_ctrl_bus_05 -of_objects [ipx::current_core]]
set_property name ctrl_bus_6 [ipx::get_bus_interfaces s_axi_ctrl_bus_06 -of_objects [ipx::current_core]]
set_property name ctrl_bus_7 [ipx::get_bus_interfaces s_axi_ctrl_bus_07 -of_objects [ipx::current_core]]
set_property name ctrl_bus_8 [ipx::get_bus_interfaces s_axi_ctrl_bus_08 -of_objects [ipx::current_core]]
set_property name ctrl_bus_9 [ipx::get_bus_interfaces s_axi_ctrl_bus_09 -of_objects [ipx::current_core]]
set_property name ctrl_bus_10 [ipx::get_bus_interfaces s_axi_ctrl_bus_10 -of_objects [ipx::current_core]]
set_property name ctrl_bus_11 [ipx::get_bus_interfaces s_axi_ctrl_bus_11 -of_objects [ipx::current_core]]
set_property name ctrl_bus_12 [ipx::get_bus_interfaces s_axi_ctrl_bus_12 -of_objects [ipx::current_core]]
set_property name ctrl_bus_13 [ipx::get_bus_interfaces s_axi_ctrl_bus_13 -of_objects [ipx::current_core]]
set_property name ctrl_bus_14 [ipx::get_bus_interfaces s_axi_ctrl_bus_14 -of_objects [ipx::current_core]]
set_property name ctrl_bus_15 [ipx::get_bus_interfaces s_axi_ctrl_bus_15 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 1} [ipx::get_bus_interfaces ctrl_bus_1 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 2} [ipx::get_bus_interfaces ctrl_bus_2 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 3} [ipx::get_bus_interfaces ctrl_bus_3 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 4} [ipx::get_bus_interfaces ctrl_bus_4 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 5} [ipx::get_bus_interfaces ctrl_bus_5 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 6} [ipx::get_bus_interfaces ctrl_bus_6 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 7} [ipx::get_bus_interfaces ctrl_bus_7 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 8} [ipx::get_bus_interfaces ctrl_bus_8 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 9} [ipx::get_bus_interfaces ctrl_bus_9 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 10} [ipx::get_bus_interfaces ctrl_bus_10 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 11} [ipx::get_bus_interfaces ctrl_bus_11 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 12} [ipx::get_bus_interfaces ctrl_bus_12 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 13} [ipx::get_bus_interfaces ctrl_bus_13 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 14} [ipx::get_bus_interfaces ctrl_bus_14 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 15} [ipx::get_bus_interfaces ctrl_bus_15 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 1} [ipx::get_ports interrupt_1 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 2} [ipx::get_ports interrupt_2 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 3} [ipx::get_ports interrupt_3 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 4} [ipx::get_ports interrupt_4 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 5} [ipx::get_ports interrupt_5 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 6} [ipx::get_ports interrupt_6 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 7} [ipx::get_ports interrupt_7 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 8} [ipx::get_ports interrupt_8 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 9} [ipx::get_ports interrupt_9 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 10} [ipx::get_ports interrupt_10 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 11} [ipx::get_ports interrupt_11 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 12} [ipx::get_ports interrupt_12 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 13} [ipx::get_ports interrupt_13 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 14} [ipx::get_ports interrupt_14 -of_objects [ipx::current_core]]
set_property enablement_dependency {$NUM_KERNELS > 15} [ipx::get_ports interrupt_15 -of_objects [ipx::current_core]]

set_property enablement_presence required [ipx::get_ports clock -of_objects [ipx::current_core]]
set_property name clk [ipx::get_bus_interfaces clock -of_objects [ipx::current_core]]
set_property value kernel_in:kernel_out:net_in:net_out:axi_mem:ctrl_bus_0:ctrl_bus_1:ctrl_bus_2:ctrl_bus_3:ctrl_bus_4:ctrl_bus_5:ctrl_bus_6:ctrl_bus_7:ctrl_bus_8:ctrl_bus_9:ctrl_bus_10:ctrl_bus_11:ctrl_bus_12:ctrl_bus_13:ctrl_bus_14:ctrl_bus_15 [ipx::get_bus_parameters ASSOCIATED_BUSIF -of_objects [ipx::get_bus_interfaces clk -of_objects [ipx::current_core]]]

ipx::update_checksums [ipx::current_core]
ipx::save_core [ipx::current_core]
set revision [get_property core_revision [ipx::current_core]]
set_property core_revision [expr $revision + 1] [ipx::current_core]
ipx::create_xgui_files [ipx::current_core]
ipx::update_checksums [ipx::current_core]
ipx::save_core [ipx::current_core]
ipx::check_integrity -quiet [ipx::current_core]
ipx::archive_core ${::env(SHOAL_PATH)}/repo/${::env(SHOAL_VIVADO_VERSION)}/${::env(SHOAL_PART_FAMILY)}/GAScore/GAScore.zip [ipx::current_core]
close_project
update_files -from_files $GAScore_path/vivado/src/GAScore/GAScore.sv -to_files $GAScore_path/src/GAScore.v -filesets [get_filesets *]
close_project
