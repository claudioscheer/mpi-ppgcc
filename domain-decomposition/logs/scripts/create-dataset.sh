#!/bin/bash

get_mean_std() {
    python -c "import sys; import numpy as np; d=[float(x.strip()) for x in sys.stdin]; m=np.mean(d); s=np.std(d); print(m, s)" < /dev/stdin
}

script_dir=$(dirname "$0")

## BUBBLE SORT ##
vector_size=1000032
bubble_sort_dat_path=$script_dir/bubble-sort.dat
rm $bubble_sort_dat_path

echo 'np "Processes - Percentage items shared" Sequential Sequential_std MPI MPI_std Speedup Efficiency "MPI broadcast" MPI_broadcast_std "Speedup broadcast" "Efficiency broadcast" "MPI interleave" MPI_interleave_std "Speedup interleave" "Efficiency interleave" "MPI interleave broadcast" MPI_interleave_broadcast_std "Speedup interleave broadcast" "Efficiency interleave broadcast"' >> $bubble_sort_dat_path

file=$script_dir/../bubble-sort/sequential-$vector_size.txt
total_time=$(cat $file | grep -P "Time\ssort\s\(ms\):" | grep -Po "[0-9]+[.][0-9]+" | get_mean_std)
sequential_time=$(echo $total_time | awk '{ print $1 }')
row="seq. seq. "
for element in $total_time; do
    row="$row$element "
done
row="$row 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0"
echo $row >> $bubble_sort_dat_path

percentage_items_shared_np=("24 .1" "24 .3" "24 .5" "48 .1" "48 .3" "48 .5")
for p_np in "${percentage_items_shared_np[@]}"; do
    percentage=$(echo $p_np | awk '{ print $2 }')
    percentage_100=$(echo "$percentage * 100" | bc -l | cut -d. -f1)
    np=$(echo $p_np | awk '{ print $1 }')
    row="$np $np-$percentage_100% 0 0 "
    tests=("o" "b" "i" "bi")
    for _test in "${tests[@]}"; do
        file=$script_dir/../bubble-sort/mpi-$np-$percentage-$vector_size-*-$_test.txt
        total_time=$(cat $file | grep -P "Time\ssort\s\(ms\):" | grep -Po "[0-9]+[.][0-9]+" | get_mean_std)
        for element in $total_time; do
            row="$row$element "
        done
        t=$(echo $total_time | awk '{ print $1 }')
        speedup=$(echo "scale=8; $sequential_time / $t" | bc -l)
        efficiency=$(echo "scale=8; $speedup / $np" | bc -l)
        row="$row$speedup $efficiency "
    done
    echo $row >> $bubble_sort_dat_path
done

# Test just to stress.
percentage_items_shared_np=("96 .1" "96 .3" "96 .5" "132 .1" "132 .3" "132 .5" "176 .1" "176 .3" "176 .5" "264 .1" "264 .3" "264 .5")
for p_np in "${percentage_items_shared_np[@]}"; do
    percentage=$(echo $p_np | awk '{ print $2 }')
    percentage_100=$(echo "$percentage * 100" | bc -l | cut -d. -f1)
    np=$(echo $p_np | awk '{ print $1 }')
    row="$np $np-$percentage_100% 0 0 0 0 0 0 "

    file=$script_dir/../bubble-sort/mpi-$np-$percentage-$vector_size-*-b.txt
    total_time=$(cat $file | grep -P "Time\ssort\s\(ms\):" | grep -Po "[0-9]+[.][0-9]+" | get_mean_std)
    for element in $total_time; do
        row="$row$element "
    done
    t=$(echo $total_time | awk '{ print $1 }')
    speedup=$(echo "scale=8; $sequential_time / $t" | bc -l)
    efficiency=$(echo "scale=8; $speedup / $np" | bc -l)
    row="$row$speedup $efficiency "

    row="$row 0 0 0 0 "

    file=$script_dir/../bubble-sort/mpi-$np-$percentage-$vector_size-*-bi.txt
    total_time=$(cat $file | grep -P "Time\ssort\s\(ms\):" | grep -Po "[0-9]+[.][0-9]+" | get_mean_std)
    for element in $total_time; do
        row="$row$element "
    done
    t=$(echo $total_time | awk '{ print $1 }')
    speedup=$(echo "scale=8; $sequential_time / $t" | bc -l)
    efficiency=$(echo "scale=8; $speedup / $np" | bc -l)
    row="$row$speedup $efficiency "
    
    echo $row >> $bubble_sort_dat_path
done
## BUBBLE SORT ##

## CREATE PLOTS ##
gnuplot bubble-sort-time.gp
gnuplot bubble-sort-speedup-efficiency.gp
## CREATE PLOTS ##
