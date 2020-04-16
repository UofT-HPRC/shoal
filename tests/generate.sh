#!/usr/bin/env bash

if [[ "$#" != 2 ]]; then
    echo "Syntax: script filename top"
    exit 1
fi

old_path=$PWD
file=$1
top=$2

basePath=$SHOAL_PATH/tests
projectPath=$basePath/projects/$SHOAL_HLS_VERSION
solutionPath=$projectPath/$file/$SHOAL_PART_FAMILY
ipPath=$solutionPath/impl/ip
repoPath=$SHOAL_PATH/repo/$SHOAL_VIVADO_VERSION/$SHOAL_PART_FAMILY/$file

# If the final name is not $file, errors come up in Vivado
# during implementation. I initially tried using $top but it
# doesn't work. There may be additional names I need to change
prefixedName=${file}_${file}
finalName=$file

mkdir -p $projectPath
cd $projectPath
vivado_hls -f $basePath/generate.tcl $file $top
vivado_return=$?
if [[ $vivado_return != 0 ]]; then
    exit $vivado_return
fi
cat $solutionPath/syn/report/${finalName}_csynth.rpt
mkdir -p $repoPath
rm -rf $repoPath/*
sed -i "s/\b$prefixedName\b/${file}/g" $ipPath/run_ippack.tcl
sed -i "s/set DisplayName \"${file^}_${finalName}\"/set DisplayName \"${finalName}\"/g" \
    $ipPath/run_ippack.tcl
sed -i "s/\b$prefixedName\b/${finalName}/g" \
    $ipPath/hdl/vhdl/${prefixedName}.vhd
sed -i "s/\b$prefixedName\b/${finalName}/g" \
    $ipPath/hdl/verilog/${prefixedName}.v
sed -i "s/\b$prefixedName\b/${finalName}/g" \
    $ipPath/bd/bd.tcl
mv $ipPath/hdl/vhdl/${prefixedName}.vhd \
    $ipPath/hdl/vhdl/${finalName}.vhd
mv $ipPath/hdl/verilog/${prefixedName}.v \
    $ipPath/hdl/verilog/${finalName}.v
cd $ipPath
./pack.sh
zipFile=xilinx_com_hls_${file}_1_0.zip
cp $ipPath/$zipFile $repoPath
cd $repoPath
unzip -o $zipFile -d .
rm $zipFile
cd $old_path

# call IP specific script if it exists
if [[ -f "$basePath/$file.sh" ]]; then
    $basePath/$file.sh
fi
