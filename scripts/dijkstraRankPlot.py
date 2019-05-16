#!/usr/bin/env python3
import sys
import matplotlib.pyplot as plt
import numpy as np

inFileName = sys.argv[1]
rankOffset = 6
timesEH = []
timesCH = []
with open(inFileName) as f:
    for line in f:
        if line.startswith("timing: "):
            words = line.split()
            rank = int(words[1])
            timeEH = int(words[2])
            timeCH = int(words[3])

            while len(timesEH) < rank - rankOffset + 1:
                timesEH.append([])

            timesEH[rank - rankOffset].append(timeEH/1000)

            while len(timesCH) < rank - rankOffset + 1:
                timesCH.append([])

            timesCH[rank - rankOffset].append(timeCH/1000)


ticks = [r"$2^{" + str(i + rankOffset) + "}$" for i in range(len(timesEH))]
def set_box_color(bp, color):
    plt.setp(bp['boxes'], color=color)
    plt.setp(bp['whiskers'], color=color)
    plt.setp(bp['caps'], color=color)
    plt.setp(bp['medians'], color=color)

plt.figure()

bpl = plt.boxplot(timesEH, positions=np.array(range(len(timesEH)))*2.0-0.4, sym='', widths=0.6)
bpr = plt.boxplot(timesCH, positions=np.array(range(len(timesCH)))*2.0+0.4, sym='', widths=0.6)
set_box_color(bpl, '#D7191C') # colors are from http://colorbrewer2.org/
set_box_color(bpr, '#2C7BB6')

# draw temporary red and blue lines and use them to create a legend
plt.plot([], c='#D7191C', label='Edge Hierarchy')
plt.plot([], c='#2C7BB6', label='Contraction Hierarchy')
plt.legend()
plt.xticks(range(0, len(ticks) * 2, 2), ticks)
plt.xlim(-2, len(ticks)*2)
plt.ylabel("time [μs]")
plt.xlabel(r"Dijkstra Rank")
plt.show()
