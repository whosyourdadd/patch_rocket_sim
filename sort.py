#!/usr/bin/env python
import sys
import csv 
import operator
data = csv.reader(open('log.csv', 'rb'),delimiter=',')
sortedlist = sorted(data, key=operator.itemgetter(0))
with open("sorted_log.csv", "wb") as f:
        fileWriter = csv.writer(f, delimiter=',')
        for row in sortedlist:
                fileWriter.writerow(row)
