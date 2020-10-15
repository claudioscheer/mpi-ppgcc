reset

set terminal pdf enhanced
set output "linear-regression-speedup-efficiency.pdf"

# set title "Linear Regression: Speedup x Efficiency" offset 0,-.5
set xlabel "Number Workers"
set ylabel "Speedup"
set y2label "Efficiency"

set key box
set key outside top center vertical maxrows 1 maxcols 6
set key autotitle columnheader
set key samplen 3 spacing 1 font ",9"

set grid

set y2tics
set auto x

plot "linear-regression.dat" every ::1 using 0:13:xticlabels(1) title col with linespoints linewidth 1 linecolor "#00c000" pointtype 3 pointsize .5, \
         "linear-regression.dat" every ::1 using 0:14:xticlabels(1) title col with linespoints axes x1y2 linewidth 1 linecolor "#e69f00" pointtype 5 pointsize .3
