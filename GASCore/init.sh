#!/bin/bash

GIT_PATH="/home/sharm294/Documents/masters/git_repos/shoal"
VIVADO_PATH="/media/sharm294/HDD_1TB/Xilinx/Vivado_HLS/2017.2/include"

if [[ -n "${SHOAL_PATH}" ]]; then
  echo "SHOAL_PATH already exists as an environment variable. Pls handle"
  exit
fi

if [[ -n "${SHOAL_VIVADO_HLS}" ]]; then
  echo "SHOAL_VIVADO_HLS already exists as an environment variable. Pls handle"
  exit
fi

if [[ -f ${GIT_PATH}/.initialized ]]; then
  echo "Initialization already run!"
  exit
fi

touch ${GIT_PATH}/.initialized

GASCORE_PATH=${GIT_PATH}/GASCore
SHARE_PATH=${GIT_PATH}/share

mkdir -p $GASCORE_PATH/build
mkdir -p $GASCORE_PATH/build/bin
mkdir -p $SHARE_PATH/build
mkdir -p $SHARE_PATH/build/bin
mkdir -p $GASCORE_PATH/repo
mkdir -p $GASCORE_PATH/testbench/build

echo "" >> ~/.bashrc
echo "#--- Begin: added by SHOAL ---#" >> ~/.bashrc
echo "export SHOAL_PATH=$GIT_PATH" >> ~/.bashrc
echo "export SHOAL_VIVADO_HLS=$VIVADO_PATH" >> ~/.bashrc
echo "" >> ~/.bashrc

echo "if [[ -z "$PYTHONPATH" ]]; then" >> ~/.bashrc
echo "  export PYTHONPATH=$SHOAL_PATH/share" >> ~/.bashrc
echo "else" >> ~/.bashrc
echo "  for x in $SHOAL_PATH/share; do" >> ~/.bashrc
echo "    case ":$PYTHONPATH:" in" >> ~/.bashrc
echo "      *":$x:"*) :;; # already there" >> ~/.bashrc
echo "      *) PYTHONPATH="$x:$PYTHONPATH";;" >> ~/.bashrc
echo "    esac" >> ~/.bashrc
echo "  done" >> ~/.bashrc
echo "fi" >> ~/.bashrc
echo "#--- End: added by SHOAL ---#" >> ~/.bashrc

source ~/.bashrc
