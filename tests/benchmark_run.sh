#!/usr/bin/env bash

print_to_terminal=0
run_separate=0
file_name=hw-hw-diff
protocol=tcp
modes=("throughput")
payloads=( 0 1 2 3 4 5 6 7 8 9 )
# payloads=( 9 )
test_nonpayload=( "short" "strided" "vector" )
# test_nonpayload=()
tests_payload=( "medium" "medium_fifo" "long" "long_fifo" )
# tests_payload=("long")
iterations=1000
exe="sudo ./tests/build/bin/benchmark"

cd $SHOAL_PATH
# make lib DEBUG=0
# rm -f $SHOAL_PATH/tests/build/benchmark.d $SHOAL_PATH/tests/build/benchmark.o $SHOAL_PATH/tests/build/benchmark_main.d $SHOAL_PATH/tests/build/benchmark_main.o
# make galapagos-benchmark K_START=0 K_END=2 MODE=x86 DEBUG=0

if [[ $run_separate == 1 ]]; then
    for mode in ${modes[@]}; do
        data_file=data/ubench_${file_name}_${mode}_${protocol}_${iterations}.txt
        rm -f $data_file

        for test in "${tests_payload[@]}"; do
            for payload in "${payloads[@]}"; do
                python tests/benchmark_data.py ${mode} --test $test --payload_min $payload --payload_max $((payload+1)) --iterations ${iterations}
                if [[ $print_to_terminal == 1 ]]; then
                    $exe
                else
                    $exe >> $data_file
                fi
            done
        done

        for test in "${test_nonpayload[@]}"; do
            python tests/benchmark_data.py ${mode} --test $test --iterations ${iterations}
            if [[ $print_to_terminal == 1 ]]; then
                $exe
            else
                $exe >> $data_file
            fi
        done
    done
else
    for mode in ${modes[@]}; do
        # python tests/benchmark_data.py ${mode} --iterations ${iterations}
        # python tests/benchmark_data.py --test long --payload_min 9 --payload_max 10 --iterations ${iterations} throughput
        data_file=data/ubench_${file_name}_${mode}_${protocol}_${iterations}.txt
        rm -f *.log
        if [[ $print_to_terminal == 1 ]]; then
            $exe
        else
            rm -f $data_file
            $exe >> $data_file
        fi
    done
fi

