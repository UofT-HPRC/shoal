#!/usr/bin/env bash

help_message="
run.sh MODE [args]

vivado
  project mode sim create
    project: name of the project
    mode: batch | gui | tcl
    sim: 0 | behav
    create: 0 | 1

package
  [no args]

export
  make_hw make_GAScore
    make_hw: generate HLS IP from scratch (0 | 1)
    make_GASCore: generate GAScore from scratch (0 | 1)
"

if [[ "$#" == 0 || $1 == '--h' || $1 == '-help' ]]; then
  echo -e "$help_message"
  exit 1
fi

if [[ $1 == 'vivado' && "$#" == 5 ]]; then
  make sim-$2 VIV_MODE=$3 VIV_SIM=$4 VIV_CREATE=$5
elif [[ $1 == 'package' && "$#" == 1 ]]; then
  make package
elif [[ $1 == 'export' && "$#" == 3 ]]; then
  if [[ $2 == 1 ]]; then
    make hw
  fi
  make sim-GAScore VIV_MODE=batch VIV_SIM=0 VIV_CREATE=$3
  make package
else
  echo -e "$help_message"
  exit 1
fi
