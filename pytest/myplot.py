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
    speedup = np.array([])
    for time in elapsedTimes:
        speedup = np.append(speedup, np.array(
            [round(time[0]/data, 4) for data in time[1:]]))
    num_particles = [config[2] for config in args]
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
    times = [0.04, 0.04, 0.04, 0.02, 0.08, 0.05, 0.11,
             0.22, 0.11, 0.15, 0.12, 0.16, 0.13, 0.17]
    times_config = tuple([tuple(times[:7]), tuple(times[7:])])
    print(times_config)
#     getPlot(
        # times_config,
        # configurations
#     )
    
    single_times, pthread_times, openmp_times = [], [], []
    
    for i in range(0, len(times)):
        if configurations[i][-1] == 'dla_single_thread':
                single_times.append((times[i], configurations[i][2]))
        elif configurations[i][-1] == 'dla_pthread':
                pthread_times.append((times[i], configurations[i][2],  configurations[i][3]))
        else:
                openmp_times.append((times[i], configurations[i][2],  configurations[i][3]))
    
    s = ""
    c =  [73.95, 118.72, 148.87, 113.53, 141.57, 125.68, 147.69, 328.45, 209.75, 238.57, 238.93, 270.59, 270.77, 300.42] 
    for i in c:
            s += f"{i} "
    print(s)
    #getBar(configurations, single_times, pthread_times, openmp_times)
    pass
