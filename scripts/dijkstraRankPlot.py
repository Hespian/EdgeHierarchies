#!/usr/bin/env python3
import sys
import matplotlib.pyplot as plt
import numpy as np

inFileName = sys.argv[1]
rankOffset = 6
timesEH = []
verticesEH = []
edgesEH = []
timesCH = []
verticesCH = []
edgesCH = []
with open(inFileName) as f:
    for line in f:
        if line.startswith("result EH: "):
            words = line.split()
            rank = int(words[2])
            time = int(words[3])
            vertices = int(words[4])
            edges = int(words[5])

            while len(timesEH) < rank - rankOffset + 1:
                timesEH.append([])
            while len(verticesEH) < rank - rankOffset + 1:
                verticesEH.append([])
            while len(edgesEH) < rank - rankOffset + 1:
                edgesEH.append([])

            timesEH[rank - rankOffset].append(time/1000)
            edgesEH[rank - rankOffset].append(edges)
            verticesEH[rank - rankOffset].append(vertices)

        if line.startswith("result CH: "):
            words = line.split()
            rank = int(words[2])
            time = int(words[3])
            vertices = int(words[4])
            edges = int(words[5])

            while len(timesCH) < rank - rankOffset + 1:
                timesCH.append([])
            while len(verticesCH) < rank - rankOffset + 1:
                verticesCH.append([])
            while len(edgesCH) < rank - rankOffset + 1:
                edgesCH.append([])

            timesCH[rank - rankOffset].append(time/1000)
            edgesCH[rank - rankOffset].append(edges)
            verticesCH[rank - rankOffset].append(vertices)


def set_box_color(bp, color):
    plt.setp(bp['boxes'], color=color)
    plt.setp(bp['whiskers'], color=color)
    plt.setp(bp['caps'], color=color)
    plt.setp(bp['medians'], color=color)


def plot(dataEH, dataCH, title, ylabel):

    ticks = [r"$2^{" + str(i + rankOffset) + "}$" for i in range(len(dataEH))]

    plt.figure()

    bpl = plt.boxplot(dataEH, positions=np.array(range(len(dataEH)))*2.0-0.4, sym='', widths=0.6)
    bpr = plt.boxplot(dataCH, positions=np.array(range(len(dataCH)))*2.0+0.4, sym='', widths=0.6)
    set_box_color(bpl, '#D7191C') # colors are from http://colorbrewer2.org/
    set_box_color(bpr, '#2C7BB6')

    # draw temporary red and blue lines and use them to create a legend
    plt.plot([], c='#D7191C', label='Edge Hierarchy')
    plt.plot([], c='#2C7BB6', label='Contraction Hierarchy')
    plt.legend()
    plt.xticks(range(0, len(ticks) * 2, 2), ticks)
    plt.xlim(-2, len(ticks)*2)
    plt.ylabel(ylabel)
    plt.xlabel(r"Dijkstra Rank")
    plt.title(title)
    plt.show()
    plt.clf()

plot(timesEH, timesCH, "Time", "time [μs]")
plot(verticesEH, verticesCH, "Vertices settled", "#vertices settles")
plot(edgesEH, edgesCH, "Edges relaxed", "#edges relaxed")
