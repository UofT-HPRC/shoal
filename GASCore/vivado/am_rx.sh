#!/bin/bash
#TODO error out if symlink source file doesn't exist

GASCore_path=$SHOAL_PATH/GASCore
vivado_path=$GASCore_path/vivado
old_path=$PWD
file="am_rx"

mkdir -p $vivado_path/projects
mkdir -p $vivado_path/src
mkdir -p $vivado_path/src/$file

link_path=$vivado_path/src/$file
ln -sf $GASCore_path/repo/$file/hdl/verilog/* $link_path
ln -sf $GASCore_path/testbench/build/${file}_tb.sv $link_path/${file}_tb.sv
ln -sf $GASCore_path/testbench/build/${file}_sv.dat $link_path/${file}_sv.dat

cd $vivado_path/projects