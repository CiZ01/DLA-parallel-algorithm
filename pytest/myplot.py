#!/bin/python3

import numpy as np
import matplotlib.pyplot as plt

'''
args = (n,m,num_particles, threads, n1, m1, num_particles1, threads1, ...)
'''
def getPlot(numTests : int, elapsedTimes : list, args : tuple[str]):
        plt.figure(figsize=(10, 8))
        x_pos = np.arange(len(args))
        y_pos = tuple([time for time in elapsedTimes])
        plt.plot(x_pos,y_pos, data=elapsedTimes, marker='o', color='b')
        plt.xticks(x_pos, args, rotation=30, fontsize=8, ha='right')
        plt.yticks([int(y) for y in np.linspace(0, max(y_pos))])
        plt.ylabel('Elapsed time (s)')
        plt.xlabel('Tests')
        plt.title('Elapsed time for each test')
        plt.savefig('test_plot.png')
        #plt.show()
        return