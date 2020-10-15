reset

set terminal pdf enhanced
set output "linear-regression-time.pdf"

set title "Linear Regression Execution Time" offset 0,-.5
set xlabel "Number Workers"
set ylabel "Time (s)"
set y2label "Efficiency"

set key box
set key outside top center vertical maxrows 2 maxcols 6
set key autotitle columnheader
set key samplen 3 spacing 1 font ",9"

set style data histogram
set style histogram cluster gap 2 errorbars linewidth 1

set y2tics

set auto x

plot "linear-regression.dat" using 2:3:xticlabels(1) title col linewidth 2 linecolor "#ff0000" fillstyle pattern 1, \
         "linear-regression.dat" using 4:5:xticlabels(1) title col linewidth 2 linecolor "#00c000" fillstyle pattern 6, \
         "linear-regression.dat" using 6:7:xticlabels(1) title col linewidth 2 linecolor "#4169e1" fillstyle pattern 4, \
         "linear-regression.dat" using 8:9:xticlabels(1) title col linewidth 2 linecolor "#8b0000" fillstyle pattern 2, \
         "linear-regression.dat" using 10:11:xticlabels(1) title col linewidth 2 linecolor "#556b2f" fillstyle pattern 5, \
         "linear-regression.dat" using 0:14 title col linewidth 1 linecolor "red" with lines axes x1y2, \
