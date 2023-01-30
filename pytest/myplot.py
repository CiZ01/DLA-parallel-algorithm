#!/bin/python3
''''
Speedup del sistema parallelo: Sp = T1/Tp
Efficienza del sistema parallelo: Ep = Sp/p
Overhead del sistema parallelo: Oh = pTp - T1

p = num_threads

- Lo speedup misura la riduzione del tempo di esecuzione rispetto
all'algoritmo su un processore. Nel caso ideale Sp = p;

- L'efficienza misura quanto l'algoritmo sfrutta il parallelismo del
calcolatore. Nel caso ideale Efficienza = 1;

- L'overhead misura quanto lo speedup differisce da quello ideale, ovvero
rappresenta un'indicazione sullo spreco delle risorse di calcolo. 
Idealmente Oh = 0.
'''
import numpy as np
import matplotlib.pyplot as plt

EXE_FILENAMES = ("dla_single_thread", "dla_pthread", "dla_openmp")


'''
args = (n,m,num_particles, threads, n1, m1, num_particles1, threads1, ...)
'''
def calc_value():
        return



def getPlot(elapsedTimes: list, args: tuple[str]):
    #SPEEDUP
    speedup = np.array([])
    for time in elapsedTimes:
        speedup = np.append(speedup, np.array(
            [round(time[0]/data, 4) for data in time[1:]]))
        
    percent = [int(config[2]/(config[0]*config[1])*100) for config in args]
    plt.figure(figsize=(10, 8))
    x_pos = np.array([config[2] for config in args[1:-1]])
    y_pos = tuple([1/x for x in speedup])
    #plt.plot(x_pos, y_pos, data=speedup, marker='o',
     #        color='b')
    print(percent)
    
    _, ax = plt.subplots()
    ax.plot(x_pos, y_pos, data=speedup, marker='o', color='b')
    
    for i, txt in enumerate(x_pos):
        ax.annotate(f"{percent[i]}%", (x_pos[i], y_pos[i]))
    plt.xticks(x_pos, fontsize=8, ha='right')
    plt.yticks(y_pos, fontsize=8)
    plt.ylabel('Speedup')
    plt.xlabel('Numero di particelle')
    plt.title('DLA \n Speedup in funzione al numero di particelle')
    plt.savefig('test_plot.png')
    return


def getBar(configurations : list, single_times: list, pthread_times: list, openmp_times: list):

	x = np.array([config[2] for config in configurations])
	s_time = np.array([time[0] for time in single_times])
	p_time = np.array([time[0] for time in pthread_times])
	o_time = np.array([time[0] for time in openmp_times])

	offset = 0.2
	for i in range(0, len(2)):
			#single
			plt.bar(x-offset, s_time[i])
			#pthread
			plt.bar(x, p_time[i])
			#openmp
			plt.bar(x+offset, o_time[i])
			
	plt.savefig('test_bar.png')
	return

def inList(s : str):
    s = s.replace('\t', ' ')
    s = s.replace(',', '.')
    s = ' '.join(s.split())
    speedup = [float(y) for y in s.split(' ')]
    p4, p8, p16, o4, o8, o16 = [],[],[],[],[],[]
    for i in range(0, len(speedup), 6):
        p4.append(speedup[i])
        o4.append(speedup[i+1])
        p8.append(speedup[i+2])
        o8.append(speedup[i+3])
        p16.append(speedup[i+4])
        o16.append(speedup[i+5])
        
    speedup = [p4, o4, p8, o8, p16, o16]
    return speedup


def getPlot2(speedup : list , x_pos : list, percent : list):
    speedup_np = np.array(speedup)
    print(speedup_np)
    m = int(np.max(speedup_np))
    y_pos = np.arange(m-0.1, m+1, 0.05)
    x_pos_np = np.array(x_pos)
	
	
    _, ax = plt.subplots()
 
    #p4
    plt.plot(x_pos, speedup_np[0], marker='o', color='b') 
    #o4
    plt.plot(x_pos, speedup_np[1], marker='o', color='r')
    #p8
    plt.plot(x_pos, speedup_np[2], marker='o', color='g')
    #o8
    plt.plot(x_pos, speedup_np[3], marker='o', color='y')
    #p16
    plt.plot(x_pos, speedup_np[4], marker='o', color='c')
    #o16
    plt.plot(x_pos, speedup_np[5], marker='o', color='m')
        
    
    
    plt.xticks(x_pos, fontsize=8, ha='center')
    plt.yticks(y_pos, fontsize=8)
    
    plt.legend(['pthread 4 threads', 'openMP 4 threads',
                'pthread 8 threads', 'openMP 8 threads',
                'pthread 16 threads', 'openMP 16 threads'])

    plt.ylabel('Speedup')
    plt.xlabel('Numero di particelle')
    plt.title('DLA \n Speedup in funzione al numero di particelle')
    plt.savefig('test_plot.png')
    return


if __name__ == '__main__':
    configurations = [
        # (n, m, num_particles, num_threads, iters, filename)
        (100, 100, 400, 0, 1000, EXE_FILENAMES[0]),
        (100, 100, 400, 4, 1000,  EXE_FILENAMES[1]),
        (100, 100, 400, 4, 1000, EXE_FILENAMES[2]),
        (100, 100, 400, 8, 1000, EXE_FILENAMES[1]),
        (100, 100, 400, 8, 1000, EXE_FILENAMES[2]),
        (100, 100, 400, 16, 1000, EXE_FILENAMES[1]),
        (100, 100, 400, 16, 1000, EXE_FILENAMES[2]),
        (100, 100, 700, 0, 1000, EXE_FILENAMES[0]),
        (100, 100, 700, 4, 1000, EXE_FILENAMES[1]),
        (100, 100, 700, 4, 1000, EXE_FILENAMES[2]),
        (100, 100, 700, 8, 1000, EXE_FILENAMES[1]),
        (100, 100, 700, 8, 1000, EXE_FILENAMES[2]),
        (100, 100, 700, 16, 1000, EXE_FILENAMES[1]),
        (100, 100, 700, 16, 1000, EXE_FILENAMES[2]),
    ]
	
    s = '2,22	2,12	2,35	2,33	2,35	2,28 2,42	2,48	2,43	2,48	2,42	2,44 2,40	2,55	2,38	2,48	2,38	2,51 2,21	2,49	2,11	2,45	2,18	2,38 2,15	2,53	1,93	2,53	2,02	2,48'
    speedup = inList(s)
    num_p = [i*100000 for i in range(1,10,2)]
    getPlot2(speedup, num_p, [])
	
    pass
