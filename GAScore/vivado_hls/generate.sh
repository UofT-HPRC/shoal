#!/bin/bash

if [[ "$#" != 1 ]]; then
    echo "Syntax: script moduleName"
    exit 1
fi

old_path=$PWD
file=$1

projectPath=$SHOAL_PATH/GAScore/vivado_hls/projects
solutionPath=$projectPath/$file/Virtex_Ultrascale
ipPath=$solutionPath/impl/ip
prefixedName=${file}_${file}

mkdir -p $projectPath
cd $projectPath
vivado_hls -f ../generate.tcl $file
vivado_return=$?
if [[ $vivado_return != 0 ]]; then
    exit $vivado_return
fi
cp $SHOAL_PATH/GAScore/src/$file.cpp $solutionPath
cat $solutionPath/syn/report/${file}_csynth.rpt
repoPath=$SHOAL_PATH/GAScore/repo/$file
mkdir -p $repoPath
rm -rf $repoPath/*
# sed -i "s/set IPName      \"$prefixedName\"/set IPName      \"${file}\"/g" \
#     $ipPath/run_ippack.tcl
sed -i "s/\b$prefixedName\b/${file}/g" $ipPath/run_ippack.tcl
sed -i "s/set DisplayName \"${file^}_${file}\"/set DisplayName \"${file}\"/g" \
    $ipPath/run_ippack.tcl
sed -i "s/\b$prefixedName\b/${file}/g" \
    $ipPath/hdl/vhdl/${file}_${file}.vhd
sed -i "s/\b$prefixedName\b/${file}/g" \
    $ipPath/hdl/verilog/${file}_${file}.v
sed -i "s/\b$prefixedName\b/${file}/g" \
    $ipPath/bd/bd.tcl
mv $ipPath/hdl/vhdl/${file}_${file}.vhd \
    $ipPath/hdl/vhdl/${file}.vhd
mv $ipPath/hdl/verilog/${file}_${file}.v \
    $ipPath/hdl/verilog/${file}.v
cd $ipPath 
./pack.sh
zipFile=xilinx_com_hls_${file}_1_0.zip
cp $ipPath/$zipFile $repoPath
cd $repoPath
unzip -o $zipFile -d .
rm $zipFile
cd $old_path
