#!/bin/bash

old_path=$PWD
file="xpams"

mkdir -p $SHOAL_PATH/GASCore/vivado_hls/projects
cd $SHOAL_PATH/GASCore/vivado_hls/projects
vivado_hls -f ../$file.tcl
mkdir -p $SHOAL_PATH/GASCore/repo/$file
cp $SHOAL_PATH/GASCore/vivado_hls/projects/$file/Virtex_Ultrascale/impl/ip/\
xilinx_com_hls_${file}_1_0.zip $SHOAL_PATH/GASCore/repo/$file/
cd $SHOAL_PATH/GASCore/repo/$file/
unzip -o xilinx_com_hls_${file}_1_0.zip -d .
rm xilinx_com_hls_${file}_1_0.zip
cd $old_path
