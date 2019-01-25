#!/usr/bin/env bash
#TODO error out if symlink source file doesn't exist

if [[ "$#" != 2 ]]; then
    echo "Syntax: script moduleName type"
    echo "type: hdl or c"
    exit 1
fi

GAScore_path=$SHOAL_PATH/GAScore
vivado_path=$GAScore_path/vivado
src_path=$GAScore_path/src
build_path=$GAScore_path/testbench/build
file=$1
type=$2
if [[ $type != "hdl" && $type != "c" ]]; then
    echo "type must be hdl or c"
    exit 1
fi

mkdir -p $vivado_path/projects/$SHOAL_VIVADO_VERSION/$SHOAL_PART
mkdir -p $vivado_path/src
mkdir -p $vivado_path/src/$file

link_path=$vivado_path/src/$file
rm -rf $link_path/*
if [[ $type == "c" ]]; then
    ln -sf $GAScore_path/repo/$SHOAL_VIVADO_VERSION/$SHOAL_PART_FAMILY/$file/hdl/verilog/* $link_path
else
    for i in $src_path/${file}*.sv; do
        [ -f "$i" ] || break
        ln -sf -t $link_path $i
    done
    for i in $src_path/${file}*.v; do
        [ -f "$i" ] || break
        ln -sf -t $link_path $i
    done
    for i in $src_path/${file}*.vhd; do
        [ -f "$i" ] || break
        ln -sf -t $link_path $i
    done
fi
ln -sf -t $link_path $build_path/${file}/${file}_tb.sv
ln -sf -t $link_path $build_path/${file}/${file}_sv.dat
for i in $build_path/${file}/*.tcl; do
    [ -f "$i" ] || break
    ln -sf -t $link_path $i
done
cd $vivado_path/projects/$SHOAL_VIVADO_VERSION/$SHOAL_PART
