reset

set terminal pdf enhanced
set output "bubble-sort-time.pdf"

set title "Bubble Sort: Execution Time x Efficiency" offset 0,-.5
set xlabel "Number Workers"
set ylabel "Execution Time (s)"
set y2label "Efficiency"

set key box
set key outside top center vertical maxrows 2 maxcols 6
set key autotitle columnheader
set key samplen 3 spacing 1 font ",9"

set grid
set style data histogram
set style histogram cluster gap 2 errorbars linewidth 1

set y2tics
set auto x

plot "bubble-sort.dat" using 2:3:xticlabels(1) title col linewidth 2 linecolor "#ff0000" fillstyle pattern 1, \
         "bubble-sort.dat" using 4:5:xticlabels(1) title col linewidth 2 linecolor "#8b0000" fillstyle pattern 6, \
         "bubble-sort.dat" using 0:7 title col with linespoints axes x1y2 linewidth 1 linecolor "#e69f00" pointtype 5 pointsize .3
