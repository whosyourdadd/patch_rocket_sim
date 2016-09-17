#!/usr/bin/env gnuplot

set output "curr_size.jpg"

set key top right
set grid xtics ytics

set title 'Memory Profile of Test Program'
set xlabel 'Time [s]'
set ylabel 'alloc size [kb]'
set datafile separator ","
set logscale x
plot \
         "glibc_heaplog.csv" using 1:($2/1024)     title "glibc heap" with lines , \
         "ltalloc_heaplog.csv" using 1:($2/1024)   title "ltalloc heap " with lines,  
         "tcalloc_heaplog.csv" using 1:($2/1024)   title "tcalloc heap" with lines 

set terminal pdf size 28cm,18cm linewidth 2.0
set output "frag_size.jpg"

set key top right
set grid xtics ytics

set title 'Memory Profile of Test Program'
set xlabel 'Time [s]'
set ylabel 'curr frag percentage [%]'
set datafile separator ","
set logscale x
plot \
     "glibc_heaplog.csv" using 1:3   title "glibc frag" with lines , \
     "ltalloc_heaplog.csv" using 1:3   title "ltalloc frag " with lines, \
     "tcalloc_heaplog.csv" using 1:3   title "tcalloc frag" with lines 



set terminal pdf size 28cm,18cm linewidth 2.0
set output "alloc_time.jpg"

set key top right
set grid xtics ytics

set title 'Memory Profile of Test Program'
set xlabel 'Time [s]'
set ylabel 'alloc Time [ms]'
set datafile separator ","
set logscale x
plot \
     "glibc_heaplog.csv" using 1:4   title "glibc alloc time" with lines ,\
     "ltalloc_heaplog.csv" using 1:4   title "ltalloc alloc time " with lines , \
     "tcalloc_heaplog.csv" using 1:4   title "tcalloc alloc time" with lines 