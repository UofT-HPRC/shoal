#!/usr/bin/env bash

DEBUG=0

# make lib DEBUG=$DEBUG
python tests/benchmark_data.py --test all --payload_min 0 --payload_max 10 --iterations 1000 throughput
scp ./tests/build/benchmark_0_sw.mem ./tests/build/benchmark_1_sw.mem ./tests/build/benchmark_2_sw.mem agent-2-eth3:~/Documents/shoal/tests/build

# rm -f tests/build/benchmark.d tests/build/benchmark.o tests/build/benchmark_main.d tests/build/benchmark_main.o
# mv tests/benchmark_main.cpp tests/benchmark_main_0.cpp
# mv tests/benchmark_main_1.cpp tests/benchmark_main.cpp

# make galapagos-benchmark K_START=0 K_END=2 MODE=x86 KERN_BUILD=2 DEBUG=$DEBUG
# scp ./tests/build/bin/benchmark ./tests/build/benchmark_0_sw.mem ./tests/build/benchmark_1_sw.mem ./tests/build/benchmark_2_sw.mem agent-2-eth3:~/Documents/shoal/tests/build
# ssh agent-2-eth3 'mv /home/sharm294/Documents/shoal/tests/build/benchmark /home/sharm294/Documents/shoal/tests/build/bin'

# rm -f tests/build/benchmark.d tests/build/benchmark.o tests/build/benchmark_main.d tests/build/benchmark_main.o
# mv tests/benchmark_main.cpp tests/benchmark_main_1.cpp
# mv tests/benchmark_main_0.cpp tests/benchmark_main.cpp

# make galapagos-benchmark K_START=0 K_END=2 MODE=x86 KERN_BUILD=1 DEBUG=$DEBUG
