#!/usr/bin/env bash
#TODO error out if symlink source file doesn't exist

if [[ "$#" != 0 ]]; then
    echo "Syntax: script"
    echo "Script takes no arguments, $# given"
    echo "$0"
    exit 1
fi

file=hls_kernel

test_path=$SHOAL_PATH/tests
vivado_path=$test_path
src_path=$test_path/$file
build_path=$test_path/build

mkdir -p $vivado_path/src
mkdir -p $vivado_path/src/$file

link_path=$vivado_path/src/$file
rm -rf $link_path/*
# ln -sf -t $link_path $SHOAL_PATH/repo/$SHOAL_VIVADO_VERSION/$SHOAL_PART_FAMILY/$file/hdl/verilog/*
ln -sf -t $link_path $vivado_path/$file.sv
ln -sf -t $link_path $vivado_path/$file.tcl
ln -sf -t $link_path $vivado_path/$file*.coe
ln -sf -t $link_path $vivado_path/$file.wcfg

ln -sf -t $link_path $build_path/${file}/${file}_tb.sv
ln -sf -t $link_path $build_path/${file}/${file}_sv.dat
for i in $build_path/${file}/*.tcl; do
    [ -f "$i" ] || break
    ln -sf -t $link_path $i
done
mkdir -p $vivado_path/projects/$SHOAL_VIVADO_VERSION/$SHOAL_PART
cd $vivado_path/projects/$SHOAL_VIVADO_VERSION/$SHOAL_PART
