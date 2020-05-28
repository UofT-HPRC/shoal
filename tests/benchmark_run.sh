#!/usr/bin/env bash

print_to_terminal=0
run_separate=1
file_name=sw-sw-same-long-detail-no-busy
modes=("throughput")
# payloads=( 0 1 2 3 4 5 6 7 8 9 )
payloads=( 9 )
# test_nonpayload=( "short" "strided" "vector" )
test_nonpayload=()
# tests_payload=( "medium" "medium_fifo" "long" "long_fifo" )
tests_payload=("long" "long_fifo")
iterations=100000

cd $SHOAL_PATH
# make lib DEBUG=0
# rm -f $SHOAL_PATH/tests/build/benchmark.d $SHOAL_PATH/tests/build/benchmark.o $SHOAL_PATH/tests/build/benchmark_main.d $SHOAL_PATH/tests/build/benchmark_main.o
make galapagos-benchmark K_START=0 K_END=2 MODE=x86 DEBUG=0

if [[ $run_separate == 1 ]]; then
    for mode in ${modes[@]}; do
        rm -f data/microbenchmarks_${file_name}_${mode}_tcp_raw.txt

        for test in "${tests_payload[@]}"; do
            for payload in "${payloads[@]}"; do
                python tests/benchmark_data.py ${mode} --test $test --payload_min $payload --payload_max $((payload+1)) --iterations ${iterations}
                if [[ $print_to_terminal == 1 ]]; then
                    sudo ./tests/build/bin/benchmark
                else
                    sudo ./tests/build/bin/benchmark >> data/microbenchmarks_${file_name}_${mode}_tcp_raw.txt
                fi
            done
        done

        for test in "${test_nonpayload[@]}"; do
            python tests/benchmark_data.py ${mode} --test $test --iterations ${iterations}
            if [[ $print_to_terminal == 1 ]]; then
                sudo ./tests/build/bin/benchmark
            else
                sudo ./tests/build/bin/benchmark >> data/microbenchmarks_${file_name}_${mode}_tcp_raw.txt
            fi
        done
    done
else
    for mode in ${modes[@]}; do
        rm -f data/microbenchmarks_${file_name}_${mode}_tcp_raw.txt

        python tests/benchmark_data.py ${mode} --iterations ${iterations}
        if [[ $print_to_terminal == 1 ]]; then
            sudo ./tests/build/bin/benchmark
        else
            sudo ./tests/build/bin/benchmark >> data/microbenchmarks_${file_name}_${mode}_tcp_raw.txt
        fi
    done
fi

