#! /bin/python3

import numpy as np
import matplotlib.pyplot as plt


'''
args = (n,m,num_particles, threads, n1, m1, num_particles1, threads1, ...)
'''
def getPlot(numTests : int,elapsedTimes : list, args : tuple[str]):
        x_pos = np.arange(len(numTests))
        plt.bar(x_pos, max(elapsedTimes), align='center')
        plt.xticks(x_pos, args)
        plt.ylabel('elapsed time')
        plt.xlabel('test')
        plt.title('Elapsed time for each test')
        plt.show()
        return