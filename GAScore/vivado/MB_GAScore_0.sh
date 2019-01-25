#!/usr/bin/env bash
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
file=MB_GAScore_0

mkdir -p $vivado_path/projects/$SHOAL_VIVADO_VERSION/$SHOAL_PART

link_path=$vivado_path/src/$file
mkdir -p $link_path
rm -rf $link_path/*

ln -sf -t $link_path $vivado_path/$file.tcl

cd $vivado_path/projects/$SHOAL_VIVADO_VERSION/$SHOAL_PART
