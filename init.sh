#!/bin/bash

if [[ $# !=  2 ]]; then
    echo "Usage: init.sh /abs/path/to/shoal/repository /abs/path/to/vivado_hls/include"
    return 0
fi

if [[ -n "${SHOAL_PATH}" ]]; then
  echo "SHOAL_PATH already exists as an environment variable."
  return 0
fi

if [[ -n "${SHOAL_VIVADO_HLS}" ]]; then
  echo "SHOAL_VIVADO_HLS already exists as an environment variable."
  return 0
fi

repoPath=$1
vivadoPath=$2
configFile=~/.shoal

if [[ -f $configFile ]]; then
  echo "Initialization already run!"
  return 0
fi

touch $configFile

GASCORE_PATH=${repoPath}/GAScore

mkdir -p $GASCORE_PATH/build
mkdir -p $GASCORE_PATH/build/bin
mkdir -p $GASCORE_PATH/repo
mkdir -p $GASCORE_PATH/testbench/build

echo "" >> $configFile
echo "#--- Begin: added by SHOAL ---#" >> $configFile
echo "export SHOAL_PATH=$repoPath" >> $configFile
echo "export SHOAL_VIVADO_HLS=$vivadoPath" >> $configFile
echo "#--- End: added by SHOAL ---#" >> $configFile

echo "source $configFile #added by shoal" >> ~/.bashrc
source ~/.bashrc