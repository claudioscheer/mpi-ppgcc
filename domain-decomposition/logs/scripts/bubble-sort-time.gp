reset

set terminal pdf enhanced
set output "bubble-sort-time.pdf"

# set title "Bubble Sort: Execution Time" offset 0,-.5
set xlabel "Number Workers - Percentage Items Shared"
set ylabel "Execution Time (ms)"

set key box
set key outside top center vertical maxrows 2 maxcols 3
set key autotitle columnheader
set key samplen 3 spacing 1 font ",9"

set grid
set style data histogram
set style histogram cluster gap 2 errorbars linewidth 1

set format y "10^{%L}"
set logscale y
set auto x

plot "bubble-sort.dat" using 3:4:xticlabels(2) title col linewidth 2 linecolor "#ff0000" fillstyle pattern 1, \
         "bubble-sort.dat" using 5:6:xticlabels(2) title col linewidth 2 linecolor "#00c000" fillstyle pattern 2, \
         "bubble-sort.dat" using 9:10:xticlabels(2) title col linewidth 2 linecolor "#4169e1" fillstyle pattern 6, \
         "bubble-sort.dat" using 13:14:xticlabels(2) title col linewidth 2 linecolor "#8b0000" fillstyle pattern 4, \
         "bubble-sort.dat" using 17:18:xticlabels(2) title col linewidth 2 linecolor "#556b2f" fillstyle pattern 5
