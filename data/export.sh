#!/usr/bin/env bash

source_dir="./data/build"
export_dir="../thesis/5d47860467573e3c760ed5d5"
export_image_dir=$export_dir/images

rm -rf $export_image_dir

curr_dir=$export_image_dir/libGalapagos
mkdir -p $curr_dir

cp $source_dir/libGalapagos/no_busy/latency_baseline.png $curr_dir/
cp $source_dir/libGalapagos/no_busy/throughput_0_reply.png $curr_dir/
cp $source_dir/libGalapagos/no_busy/throughput_1_reply.png $curr_dir/
cp $source_dir/libGalapagos/cross/cross_throughput_0.png $curr_dir/
cp $source_dir/libGalapagos/cross/cross_throughput_1.png $curr_dir/

curr_dir=$export_image_dir/sw_sw_same_optimized
mkdir -p $curr_dir

cp $source_dir/sw-sw-same-optimized/tcp/throughput/efficiency/payload_throughput_0_all.png $curr_dir/
cp $source_dir/sw-sw-same-optimized/tcp/throughput/efficiency/payload_throughput_1_all.png $curr_dir/