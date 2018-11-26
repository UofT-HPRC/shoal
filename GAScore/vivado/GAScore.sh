#!/bin/bash
#TODO error out if symlink source file doesn't exist

if [[ "$#" != 0 ]]; then
    echo "Syntax: script"
    echo "Script takes no arguments"
    exit 1
fi

GAScore_path=$SHOAL_PATH/GAScore
vivado_path=$GAScore_path/vivado
src_path=$GAScore_path/src
build_path=$GAScore_path/testbench/build
file=GAScore

mkdir -p $vivado_path/projects
mkdir -p $vivado_path/src
mkdir -p $vivado_path/src/$file

link_path=$vivado_path/src/$file
rm -rf $link_path/*

ln -sf -t $link_path $GAScore_path/repo/handler/hdl/verilog/*
ln -sf -t $link_path $src_path/handler_wrapper.sv
ln -sf -t $link_path $src_path/$file.sv
ln -sf -t $link_path $vivado_path/$file.tcl

ln -sf -t $link_path $build_path/${file}/${file}_tb.sv
ln -sf -t $link_path $build_path/${file}/${file}_sv.dat
for i in $build_path/${file}/*.tcl; do
    [ -f "$i" ] || break
    ln -sf -t $link_path $i
done
cd $vivado_path/projects
