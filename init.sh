#!/usr/bin/env bash

find_family () {
  configFile=$1
  part=$2

  FPGA_family="${part:2:1}"
  if [[ $FPGA_family == 'k' ]]; then
    FPGA_family='Kintex'
  elif [[ $FPGA_family == 'v' ]]; then
    FPGA_family='Virtex'
  elif [[ $FPGA_family == '7' ]]; then
    FPGA_family='7series'
  else
    FPGA_family="${part:2:2}"
    if [[ $FPGA_family != 'zu' ]]; then
      return 1
    fi
  fi

  if [[ $FPGA_family == '7series' ]]; then
    FPGA_type="${part:3:1}"
    if [[ $FPGA_type == 'k' ]]; then
      FPGA_type='Kintex'
    elif [[ $FPGA_type == 'v' ]]; then
      FPGA_type='Virtex'
    elif [[ $FPGA_type == 'z' ]]; then
      FPGA_type='Zynq'
    else
      return 1
    fi
  else
    FPGA_type="${part%%-*}"
    FPGA_type="${FPGA_type: -1}"
    if [[ $FPGA_type == 'p' ]]; then
      FPGA_type='Ultrascale_Plus'
    else
      FPGA_type='Ultrascale'
    fi
  fi

  if [[ $FPGA_family == 'zu' ]]; then
    Solution='Zynq_Ultrascale_Plus'
  else
    Solution=${FPGA_family}_${FPGA_type}
  fi

  echo "export SHOAL_PART_FAMILY=$Solution" >> $configFile

  return 0
}

if [[ "$#" != 6 && "$#" != 7 ]]; then
  echo "6/7 arguments expected, got $#"
  echo "Usage: init.sh /abs/path/to/shoal/repository /abs/path/to/vivado /abs/path/to/vivado_hls <part name> vivado_version vivado_hls_version [board name]"
  return 1
fi

repoPath=$(readlink -f "$1")
vivadoPath=$(readlink -f "$2")
hlsPath=$(readlink -f "$3")
part=$4
vivadoVersion=$5
hlsVersion=$6
if [[ "$#" == 7 ]]; then
  board=$7
fi

configFile=~/.shoal

if [[ -f $configFile ]]; then
  echo "Warning: initialization already run! Updating environment..."
  rm $configFile
fi

if [[ $hlsVersion == "2017.2" ]]; then
  hlsPath=$hlsPath/$hlsVersion/include
else
  echo "Error: unsupported Vivado HLS Version $hlsVersion"
  return 1
fi

# https://stackoverflow.com/a/2264537
part=$(echo "$part" | tr '[:upper:]' '[:lower:]') # convert to lower-case

find_family $configFile $part
if [[ $? != 0 ]]; then
  echo "Error: Unable to identify FPGA family from part $part"
  return 1
fi

vivadoPath=$vivadoPath/$vivadoVersion

GASCORE_PATH=${repoPath}/GAScore

mkdir -p $GASCORE_PATH/build/bin
mkdir -p ${repoPath}/repo/$SHOAL_VIVADO_VERSION/$SHOAL_PART_FAMILY
mkdir -p $GASCORE_PATH/repo/$SHOAL_VIVADO_VERSION/$SHOAL_PART_FAMILY
mkdir -p $GASCORE_PATH/testbench/build
mkdir -p $SHOAL_PATH/tests/build/bin
mkdir -p $SHOAL_PATH/apps/build/bin

{
  echo "export SHOAL_PATH=$repoPath"
  echo "export SHOAL_VIVADO_PATH=$vivadoPath"
  echo "export SHOAL_HLS_PATH=$hlsPath"
  echo "export SHOAL_PART=$part"
  echo "export SHOAL_VIVADO_VERSION=$vivadoVersion"
  echo "export SHOAL_HLS_VERSION=$hlsVersion"
} >> $configFile

if [[ "$#" == 7 ]]; then
  echo "export SHOAL_BOARD=$board" >> $configFile
fi

source $configFile

if ! grep -Fq "# added by shoal" ~/.bashrc; then 
  echo "source $configFile # added by shoal" >> ~/.bashrc
fi
