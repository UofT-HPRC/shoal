#!/bin/bash
#TODO error out if symlink source file doesn't exist

if [[ "$#" != 1 ]]; then
    echo "Syntax: script moduleName"
    exit 1
fi

GASCore_path=$SHOAL_PATH/GASCore
vivado_path=$GASCore_path/vivado
old_path=$PWD
file=$1

mkdir -p $vivado_path/projects
mkdir -p $vivado_path/src
mkdir -p $vivado_path/src/$file

link_path=$vivado_path/src/$file
rm -rf $link_path/*
ln -sf $GASCore_path/repo/$file/hdl/verilog/* $link_path
ln -sf $GASCore_path/testbench/build/${file}_tb.sv $link_path/${file}_tb.sv
ln -sf $GASCore_path/testbench/build/${file}_sv.dat $link_path/${file}_sv.dat

cd $vivado_path/projects