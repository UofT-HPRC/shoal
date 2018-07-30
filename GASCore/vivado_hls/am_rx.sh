#!/bin/bash

old_path=$PWD

mkdir -p $SHOAL_PATH/GASCore/vivado_hls/projects
cd $SHOAL_PATH/GASCore/vivado_hls/projects
vivado_hls -f ../am_rx.tcl
mkdir -p $SHOAL_PATH/GASCore/repo/am_rx
cp $SHOAL_PATH/GASCore/vivado_hls/projects/am_rx/Virtex_Ultrascale/impl/ip/\
xilinx_com_hls_am_rx_1_0.zip $SHOAL_PATH/GASCore/repo/am_rx/
cd $SHOAL_PATH/GASCore/repo/am_rx/
unzip xilinx_com_hls_am_rx_1_0.zip -d .
rm xilinx_com_hls_am_rx_1_0.zip
cd $old_path
