reset

set terminal pdf enhanced
set output "bubble-sort-speedup-efficiency.pdf"

# set title "Bubble Sort: Speedup x Efficiency" offset 0,-.5
set xlabel "Number Workers"
set ylabel "Speedup"
set y2label "Efficiency"

set key box
set key outside top center vertical maxrows 1 maxcols 6
set key autotitle columnheader
set key samplen 3 spacing 1 font ",9"

set grid
set boxwidth 0.3
set style fill solid

set y2tics
set auto x

plot "bubble-sort.dat" using 7:xticlabels(1) title col with boxes axes x1y2 linecolor "#e69f00", \
         "bubble-sort.dat" using 0:6:xticlabels(1) title col with linespoints linewidth 2 linecolor "#8b0000" pointtype 3 pointsize .8
