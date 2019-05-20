#!/usr/bin/env bash
#TODO error out if symlink source file doesn't exist

if [[ "$#" != 0 ]]; then
    echo "Syntax: script"
    echo "Script takes no arguments"
    exit 1
fi

GAScore_path=$SHOAL_PATH/GAScore
vivado_path=$GAScore_path/vivado
build_path=$GAScore_path/testbench/build
file=GAScore_benchmark

mkdir -p $vivado_path/projects/$SHOAL_VIVADO_VERSION/$SHOAL_PART

link_path=$vivado_path/src/$file
mkdir -p $link_path
rm -rf $link_path/*

ln -sf -t $link_path $vivado_path/$file.tcl
ln -sf -t $link_path $vivado_path/${file}_prologue.tcl
ln -sf -t $link_path $GAScore_path/testbench/GAScore_benchmark.wcfg

ln -sf -t $link_path $build_path/${file}/${file}_tb.sv
ln -sf -t $link_path $build_path/${file}/${file}_sv.dat
for i in $build_path/${file}/*.tcl; do
    [ -f "$i" ] || break
    ln -sf -t $link_path $i
done
cd $vivado_path/projects/$SHOAL_VIVADO_VERSION/$SHOAL_PART
