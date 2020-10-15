reset

set terminal pdf enhanced
set output "linear-regression-speedup-efficiency.pdf"

set title "Linear Regression Speedup and Efficiency" offset 0,-.5
set xlabel "Number Workers"
set ylabel "Speedup"
set y2label "Efficiency"

set key box
set key outside top center vertical maxrows 2 maxcols 6
set key autotitle columnheader
set key samplen 3 spacing 1 font ",9"

set y2tics

set auto x

plot "linear-regression.dat" using 0:13 title col linewidth 1 linecolor "yellow" with lines, \
     "linear-regression.dat" using 0:14 title col linewidth 1 linecolor "red" with lines axes x1y2
