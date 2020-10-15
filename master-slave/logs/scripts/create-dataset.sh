#!/bin/bash

get_mean_std() {
    python -c "import sys; import numpy as np; d=[float(x.strip()) for x in sys.stdin]; m=np.mean(d); s=np.std(d); print(m, s)" < /dev/stdin
}

script_dir=$(dirname "$0")
n=3
linear_regression_dat_path=$script_dir/linear-regression.dat
rm $linear_regression_dat_path

echo "np Sequential Sequential_std Grain-10000 10000_std Grain-100000 100000_std Grain-500000 500000_std Grain-1000000 1000000_std mean_time Speedup Efficiency" >> $linear_regression_dat_path


file=$script_dir/../linear-regression/sequential-*.txt
total_time=$(cat $file | grep -P "Time\slinear\sregression\s\(s\):" | grep -Po "[0-9]+[.][0-9]+" | get_mean_std)
row="seq. "
for element in $total_time; do
    row="$row$element "
done
row="$row 0 0 0 0 0 0 0 0 0 0 0"
echo $row >> $linear_regression_dat_path

sequential_time=$(echo $total_time | awk '{ print $1 }')

processes=($(seq 2 2 24) 48)
for np in "${processes[@]}"; do
    time_grain=0
    row="$np 0 0 "
    for grain in 10000 100000 500000 1000000; do
        file=$script_dir/../linear-regression/mpi-$np-$grain-*.txt
        total_time=$(cat $file | grep -P "Time\slinear\sregression\s\(s\):" | grep -Po "[0-9]+[.][0-9]+" | get_mean_std)
        for element in $total_time; do
            row="$row$element "
        done
        t=$(echo $total_time | awk '{ print $1 }')
        time_grain=$(echo "scale=8; $time_grain + $t" | bc -l)
    done
    time_grain=$(echo "scale=8; $time_grain / 4" | bc -l)
    speedup=$(echo "scale=8; $sequential_time / $time_grain" | bc -l)
    efficiency=$(echo "scale=8; $speedup / $np" | bc -l)
    row="$row$time_grain $speedup $efficiency "
    echo $row >> $linear_regression_dat_path
done