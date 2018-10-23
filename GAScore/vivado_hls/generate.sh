#!/bin/bash

if [[ "$#" != 1 ]]; then
    echo "Syntax: script moduleName"
    exit 1
fi

old_path=$PWD
file=$1

projectPath=$SHOAL_PATH/GAScore/vivado_hls/projects
mkdir -p $projectPath
cd $projectPath
vivado_hls -f ../generate.tcl $file
vivado_return=$?
if [[ $vivado_return != 0 ]]; then
    exit $vivado_return
fi
cat $projectPath/$file/Virtex_Ultrascale/syn/report/${file}_csynth.rpt
repoPath=$SHOAL_PATH/GAScore/repo/$file
mkdir -p $repoPath
rm -rf $repoPath/*
cp $projectPath/$file/Virtex_Ultrascale/impl/ip/xilinx_com_hls_${file}_1_0.zip $repoPath
cd $repoPath
unzip -o xilinx_com_hls_${file}_1_0.zip -d .
rm xilinx_com_hls_${file}_1_0.zip
cd $old_path
