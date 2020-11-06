#!/bin/bash

get_mean_std() {
    python -c "import sys; import numpy as np; d=[float(x.strip()) for x in sys.stdin]; m=np.mean(d); s=np.std(d); print(m, s)" < /dev/stdin
}

script_dir=$(dirname "$0")

## BUBBLE SORT ##
vector_size=1000000
bubble_sort_dat_path=$script_dir/bubble-sort.dat
rm $bubble_sort_dat_path

echo "np Sequential Sequential_std MPI MPI_std Speedup Efficiency" >> $bubble_sort_dat_path

file=$script_dir/../bubble-sort/sequential-$vector_size-*.txt
# total_time=$(cat $file | grep -P "Time\ssort\s\(s\):" | grep -Po "[0-9]+[.][0-9]+" | get_mean_std)
total_time="4440 10"
sequential_time=$(echo $total_time | awk '{ print $1 }')
row="seq. "
for element in $total_time; do
    row="$row$element "
done
row="$row 0 0 0 0"
echo $row >> $bubble_sort_dat_path

delta_np=("250000 7" "125000 15" "62500 31" "31250 63" "15625 127")
for d_np in "${delta_np[@]}"; do
    delta=$(echo $d_np | awk '{ print $1 }')
    np=$(echo $d_np | awk '{ print $2 }')
    row="$np 0 0 "
    file=$script_dir/../bubble-sort/mpi-$np-$vector_size-$delta-*.txt
    total_time=$(cat $file | grep -P "Time\ssort\s\(s\):" | grep -Po "[0-9]+[.][0-9]+" | get_mean_std)
    for element in $total_time; do
        row="$row$element "
    done
    t=$(echo $total_time | awk '{ print $1 }')
    speedup=$(echo "scale=8; $sequential_time / $t" | bc -l)
    efficiency=$(echo "scale=8; $speedup / $np" | bc -l)
    row="$row$speedup $efficiency"
    echo $row >> $bubble_sort_dat_path
done
## BUBBLE SORT ##

## CREATE PLOTS ##
gnuplot bubble-sort-time.gp
gnuplot bubble-sort-speedup-efficiency.gp
## CREATE PLOTS ##
