#!/usr/bin/env bash

python data/benchmark.py 
python data/extract_jacobi.py
python data/cross_analyze.py
python data/libGalapagos.py

source_dir="./data/build"
export_dir="../thesis/5d47860467573e3c760ed5d5"
export_image_dir=$export_dir/images

rm -rf $export_image_dir

for ending in "png" "pdf"; do

curr_dir=$export_image_dir/libGalapagos
mkdir -p $curr_dir

cp $source_dir/libGalapagos/no_busy/latency_baseline.${ending} $curr_dir/
cp $source_dir/libGalapagos/no_busy/throughput_0_reply.${ending} $curr_dir/
cp $source_dir/libGalapagos/no_busy/throughput_1_reply.${ending} $curr_dir/
cp $source_dir/libGalapagos/cross/cross_throughput_0.${ending} $curr_dir/
cp $source_dir/libGalapagos/cross/cross_throughput_1.${ending} $curr_dir/
cp $source_dir/libGalapagos/cross/cross_throughput_reply.${ending} $curr_dir/
cp $source_dir/libGalapagos/no_busy_diff/throughput_0_reply.${ending} $curr_dir/throughput_0_reply_diff.${ending}
cp $source_dir/libGalapagos/no_busy_diff/throughput_1_reply.${ending} $curr_dir/throughput_1_reply_diff.${ending}

curr_dir=$export_image_dir/sw_sw_same_optimized
mkdir -p $curr_dir

cp $source_dir/sw-sw-same-optimized/tcp/throughput/10000/efficiency/payload_throughput_0_all.${ending} $curr_dir/
cp $source_dir/sw-sw-same-optimized/tcp/throughput/10000/efficiency/payload_throughput_1_all.${ending} $curr_dir/
cp $source_dir/sw-sw-same-optimized/tcp/latency/10000/medians/medians_all.${ending} $curr_dir/

curr_dir=$export_image_dir/sw_sw_diff_optimized/tcp
mkdir -p $curr_dir

cp $source_dir/sw-sw-diff-optimized/tcp/throughput/10000/efficiency/payload_throughput_0_all.${ending} $curr_dir/
cp $source_dir/sw-sw-diff-optimized/tcp/throughput/10000/efficiency/payload_throughput_1_all.${ending} $curr_dir/
cp $source_dir/sw-sw-diff-optimized/tcp/latency/10000/medians/medians_all.${ending} $curr_dir/

curr_dir=$export_image_dir/sw_sw_diff_optimized/udp
mkdir -p $curr_dir

cp $source_dir/sw-sw-diff/udp/latency/1000/medians/medians_all.${ending} $curr_dir/

curr_dir=$export_image_dir/sw_hw_optimized/tcp
mkdir -p $curr_dir

cp $source_dir/sw-hw-optimized/tcp/throughput/1000/efficiency/payload_throughput_0_all.${ending} $curr_dir/
cp $source_dir/sw-hw-optimized/tcp/throughput/1000/efficiency/payload_throughput_1_all.${ending} $curr_dir/
cp $source_dir/sw-hw-optimized/tcp/latency/1000/medians/medians_all.${ending} $curr_dir/

curr_dir=$export_image_dir/sw_hw_optimized/udp
mkdir -p $curr_dir

cp $source_dir/sw-hw-optimized/udp/throughput/1000/efficiency/payload_throughput_0_all.${ending} $curr_dir/
cp $source_dir/sw-hw-optimized/udp/throughput/1000/efficiency/payload_throughput_1_all.${ending} $curr_dir/
cp $source_dir/sw-hw-optimized/udp/latency/1000/medians/medians_all.${ending} $curr_dir/

curr_dir=$export_image_dir/hw_sw_optimized/tcp
mkdir -p $curr_dir

cp $source_dir/hw-sw-optimized/tcp/throughput/1000/efficiency/payload_throughput_0_all.${ending} $curr_dir/
cp $source_dir/hw-sw-optimized/tcp/throughput/1000/efficiency/payload_throughput_1_all.${ending} $curr_dir/
cp $source_dir/hw-sw-optimized/tcp/latency/1000/medians/medians_all.${ending} $curr_dir/

curr_dir=$export_image_dir/hw_sw_optimized/udp
mkdir -p $curr_dir

cp $source_dir/hw-sw-optimized/udp/latency/1000/medians/medians_all.${ending} $curr_dir/

curr_dir=$export_image_dir/hw_hw_same_optimized
mkdir -p $curr_dir

cp $source_dir/hw-hw-same-optimized/tcp/throughput/1000/efficiency/payload_throughput_0_all.${ending} $curr_dir/
cp $source_dir/hw-hw-same-optimized/tcp/throughput/1000/efficiency/payload_throughput_1_all.${ending} $curr_dir/
cp $source_dir/hw-hw-same-optimized/tcp/latency/1000/medians/medians_all.${ending} $curr_dir/

curr_dir=$export_image_dir/hw_hw_diff_optimized/tcp
mkdir -p $curr_dir

cp $source_dir/hw-hw-diff/tcp/throughput/1000/efficiency/payload_throughput_0_all.${ending} $curr_dir/
cp $source_dir/hw-hw-diff/tcp/throughput/1000/efficiency/payload_throughput_1_all.${ending} $curr_dir/
cp $source_dir/hw-hw-diff/tcp/latency/1000/medians/medians_all.${ending} $curr_dir/

curr_dir=$export_image_dir/hw_hw_diff_optimized/udp
mkdir -p $curr_dir

cp $source_dir/hw-hw-diff/udp/throughput/1000/efficiency/payload_throughput_0_all.${ending} $curr_dir/
cp $source_dir/hw-hw-diff/udp/throughput/1000/efficiency/payload_throughput_1_all.${ending} $curr_dir/
cp $source_dir/hw-hw-diff/udp/latency/1000/medians/medians_all.${ending} $curr_dir/

curr_dir=$export_image_dir/cross/same
mkdir -p $curr_dir

cp $source_dir/cross/libGal_vs_shoal/same/payload_throughput_0.${ending} $curr_dir/
cp $source_dir/cross/libGal_vs_shoal/same/payload_throughput_1.${ending} $curr_dir/

curr_dir=$export_image_dir/cross/diff
mkdir -p $curr_dir

cp $source_dir/cross/libGal_vs_shoal/diff/payload_throughput_0.${ending} $curr_dir/
cp $source_dir/cross/libGal_vs_shoal/diff/payload_throughput_1.${ending} $curr_dir/

curr_dir=$export_image_dir/group
mkdir -p $curr_dir

cp $source_dir/group/tcp_summary_latency.${ending} $curr_dir/
cp $source_dir/group/tcp_summary_throughput_0.${ending} $curr_dir/
cp $source_dir/group/tcp_summary_throughput_1.${ending} $curr_dir/
cp $source_dir/group/udp_vs_tcp_latency.${ending} $curr_dir/
cp $source_dir/group/udp_vs_tcp_throughput_0.${ending} $curr_dir/
cp $source_dir/group/udp_vs_tcp_throughput_1.${ending} $curr_dir/

curr_dir=$export_image_dir/jacobi
mkdir -p $curr_dir

cp $source_dir/jacobi/hw.${ending} $curr_dir/
cp $source_dir/jacobi/sw.${ending} $curr_dir/

done