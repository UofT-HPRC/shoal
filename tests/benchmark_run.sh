#!/usr/bin/env bash

file_name=hw_sw

cd $SHOAL_PATH
# make lib DEBUG=0
# make galapagos-benchmark K_START=0 K_END=2 MODE=x86 DEBUG=0
# python tests/benchmark_data.py
sudo ./tests/build/bin/benchmark > data/microbenchmarks_${file_name}_raw.txt
python tests/benchmark.py data/microbenchmarks_${file_name}_raw.txt