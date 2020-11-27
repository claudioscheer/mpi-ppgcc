reset

set terminal pdf enhanced
set output "bubble-sort-speedup-efficiency.pdf"

# set title "Bubble Sort: Speedup x Efficiency" offset 0,-.5
set xlabel "Number Workers - Percentage Items Shared"
set ylabel "Speedup"
set y2label "Efficiency"

set key box
set key outside top center vertical maxrows 5 maxcols 2
set key autotitle columnheader
set key samplen 3 spacing 1 font ",9"

set grid
set style data histogram
set style histogram cluster gap 2
set style fill solid border rgb "black"

set y2tics
set format y2 "2^{%L}"
set logscale y2 2
set logscale y 2
set auto x
set xtics rotate out

plot "bubble-sort.dat" every ::1 using 8:xticlabels(2) title col axes x1y2 linecolor "#9300d3", \
         "bubble-sort.dat" every ::1 using 12:xticlabels(2) title col axes x1y2 linecolor "#009e72", \
         "bubble-sort.dat" every ::1 using 16:xticlabels(2) title col axes x1y2 linecolor "#56b3e8", \
         "bubble-sort.dat" every ::1 using 20:xticlabels(2) title col axes x1y2 linecolor "#e69e00", \
         "bubble-sort.dat" every ::1 using 1:xticlabels(2) title "Optimal speedup" with linespoints linecolor "#8b0000" linewidth 2 pointtype 5 pointsize .5, \
         "bubble-sort.dat" every ::1 using 7:xticlabels(2) title col with linespoints linewidth 2 pointtype 3 pointsize .5 linecolor "#9300d3", \
         "bubble-sort.dat" every ::1 using 11:xticlabels(2) title col with linespoints linewidth 2 pointtype 3 pointsize .5 linecolor "#009e72", \
         "bubble-sort.dat" every ::1 using 15:xticlabels(2) title col with linespoints linewidth 2 pointtype 3 pointsize .5 linecolor "#56b3e8", \
         "bubble-sort.dat" every ::1 using 19:xticlabels(2) title col with linespoints linewidth 2 pointtype 3 pointsize .5 linecolor "#e69e00"
