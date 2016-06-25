#!/usr/bin/env gnuplot

set datafile separator ","
#set autoscale fix
set key outside right center
#set key inside left top vertical Right noreverse enhanced autotitle box lt black linewidth 1.000 dashtype solid
set term png size 800,600 #font "arial,12" #transparent nocrop enhanced size 800,600 font "arial,12"  background rgb 'white'
set output "memprofile.png"
#set samples 50, 50
set title "Memory Profile of Test Program" 
set title  font ",20" norotate
#set grid xtics ytics
set xlabel 'Timestamp [sec]'
set ylabel 'Memory Usage [Byte]'
#plot the first 1000~5000 row data
plot "<(sed -n '5000,5030p' sorted_log.csv)" using 1:4   title "current" with lines ,\
#     "<(sed -n '5000,10000p' sorted_log.csv)" using 1:3   title "Peak" with lines