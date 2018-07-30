#!/bin/bash

GIT_PATH="/home/sharm294/Documents/masters/git_repos/shoal"
VIVADO_PATH="/media/sharm294/HDD_1TB/Xilinx/Vivado_HLS/2017.2/include"

if [[ -z "${SHOAL_PATH}" ]]; then
  echo "SHOAL_PATH already exists as an environment variable. Pls handle"
  exit
fi

GASCORE_PATH=${SHOAL_PATH}/GASCore
SHARE_PATH=${SHOAL_PATH}/share

mkdir -p $GASCORE_PATH/build
mkdir -p $GASCORE_PATH/build/bin
mkdir -p $SHARE_PATH/build
mkdir -p $SHARE_PATH/build/bin
mkdir -p $GASCORE_PATH/repo
sed -i "s/VIVADO_PATH/$VIVADO_PATH/g" Makefile
echo "export SHOAL_PATH=$GIT_PATH" >> \
	~/.bashrc
source ~/.bashrc
