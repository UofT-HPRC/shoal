#!/bin/bash

if [[ $# !=  2 ]]; then
    echo "Usage: init.sh /abs/path/to/shoal/repository /abs/path/to/vivado_hls/include"
    exit 1
fi

if [[ -n "${SHOAL_PATH}" ]]; then
  echo "SHOAL_PATH already exists as an environment variable."
  exit
fi

if [[ -n "${SHOAL_VIVADO_HLS}" ]]; then
  echo "SHOAL_VIVADO_HLS already exists as an environment variable."
  exit
fi

repoPath=$1
vivadoPath=$2
configFile=~/.shoal

if [[ -f configFile ]]; then
  echo "Initialization already run!"
  exit
fi

touch configFile

GASCORE_PATH=${repoPath}/GASCore

mkdir -p $GASCORE_PATH/build
mkdir -p $GASCORE_PATH/build/bin
mkdir -p $GASCORE_PATH/repo
mkdir -p $GASCORE_PATH/testbench/build

echo "" >> configFile
echo "#--- Begin: added by SHOAL ---#" >> configFile
echo "export SHOAL_PATH=$repoPath" >> configFile
echo "export SHOAL_VIVADO_HLS=$vivadoPath" >> configFile
echo "#--- End: added by SHOAL ---#" >> configFile

source ${repoPath}/shoal-share/init.sh 1 repoPath vivadoPath

echo "source $configFile #added by shoal" >> ~/.bashrc
source ~/.bashrc