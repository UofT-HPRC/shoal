#!/usr/bin/env bash

include="-I$SHOAL_PATH/include \
    -I$SHOAL_PATH/GAScore/include \
    -isystem /mnt/shares/tools/Xilinx/Vivado/2018.1/include \
    -I/home/savi/Documents/varun/repos/galapagos/middleware/libGalapagos \
    -I/home/savi/Documents/varun/repos/galapagos/middleware/include \
    -I/home/savi/Documents/varun/repos/galapagos/util/spdlog/include"

warning_suppress="-Wno-unused-value -Wno-unused-variable -Wno-comment \
    -Wno-unknown-pragmas -Wno-unused-but-set-variable -Wno-unused-function \
    -Wno-sign-compare"

first_flags="-O3 -DLOG_LEVEL=0 -Wall"
flags="-MMD -MP -pthread -std=c++17"
last_flags="-lpthread -lrt -DKERN_BUILD=-1"

cxx="/usr/bin/g++-7"

# nodes=(2 3 5 9 17)
nodes=(9)
iterations=(1025)
# grid_sizes=(64 256 1024)
grid_sizes=(64)
data_file=$SHOAL_PATH/data/jacobi_shoal_sw.txt
repeat_until=0
save_file=0

# rm -f $save_file

for node in ${nodes[@]}; do
    for iteration in ${iterations[@]}; do
        for grid_size in ${grid_sizes[@]}; do

            rm -f $SHOAL_PATH/tests/build/jacobi*.o
            rm -f $SHOAL_PATH/tests/build/jacobi*.d

            # $cxx $first_flags $include $warning_suppress $flags \
            #     -o $SHOAL_PATH/tests/build/jacobi.o -c $SHOAL_PATH/tests/jacobi.cpp $last_flags \
            #     -DKERNEL_NUM_TOTAL=$node -DITERATIONS=$iteration -DGRID_SIZE=$grid_size

            $cxx $first_flags $include $warning_suppress $flags \
                -o $SHOAL_PATH/tests/build/jacobi_ctrl.o -c $SHOAL_PATH/tests/jacobi_ctrl.cpp $last_flags \
                -DKERNEL_NUM_TOTAL=$node -DITERATIONS=$iteration -DGRID_SIZE=$grid_size

            $cxx $first_flags $include $warning_suppress $flags \
                -o $SHOAL_PATH/tests/build/jacobi_comp.o -c $SHOAL_PATH/tests/jacobi_comp.cpp $last_flags \
                -DKERNEL_NUM_TOTAL=$node -DITERATIONS=$iteration -DGRID_SIZE=$grid_size

            $cxx $first_flags $include $warning_suppress $flags \
                -o $SHOAL_PATH/tests/build/jacobi_main.o -c $SHOAL_PATH/tests/jacobi_main.cpp $last_flags \
                -DKERNEL_NUM_TOTAL=$node -DITERATIONS=$iteration -DGRID_SIZE=$grid_size

            $cxx $first_flags $include $warning_suppress $flags \
                -o $SHOAL_PATH/tests/build/bin/jacobi \
                $SHOAL_PATH/tests/build/jacobi_comp.o \
                $SHOAL_PATH/tests/build/jacobi_ctrl.o \
                $SHOAL_PATH/tests/build/jacobi_main.o \
                -Wl,--wrap=kern0 -Wl,--wrap=kern1 -Wl,--wrap=kern2 \
                -L/usr/local/lib -lboost_thread -lboost_system -lpthread \
                -L$SHOAL_PATH/build -lTHeGASnet -lGalapagos

            # for i in $(seq 0 $repeat_until); do
            #     if [[ $save_file == 0 ]]; then
            #         sudo $SHOAL_PATH/tests/build/bin/jacobi
            #     else
            #         sudo $SHOAL_PATH/tests/build/bin/jacobi >> $data_file
            #     fi
            # done
        done
    done
done