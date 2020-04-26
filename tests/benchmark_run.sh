#!/usr/bin/env bash

file_name=sw-hw

cd $SHOAL_PATH
# make lib DEBUG=0
# make galapagos-benchmark K_START=0 K_END=2 MODE=x86 DEBUG=0
# python tests/benchmark_data.py
sudo ./tests/build/bin/benchmark > data/microbenchmarks_${file_name}_tcp_raw.txt
# python tests/benchmark.py data/