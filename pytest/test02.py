#!/bin/python3

import subprocess
import myplot as plot

EXE_FILENAMES = ("dla_single_thread", "dla_pthread", "dla_openmp")
numTests = 10
times = []

def run_test(configurations : list, exe : str) -> tuple:
    totalTime = 0
    for config in configurations:
        n, m, num_particles, num_threads, iters = config
        
        if num_threads > 0 and exe != "dla_single_thread":
            args = f"{n},{m}|{num_particles}|-n{num_threads}|-t{iters}".split("|")
        else:
            args = f"{n},{m}|{num_particles}|-t{iters}".split("|")

        cmd_running = [f"./{exe}.out"] + args
        
        for i in range(numTests):
            print(f"Test {i} di {numTests} - {exe} -> {config}")
            
            running = subprocess.check_call(
                cmd_running,
                stdout=open("output/out.txt", "w"),
                stderr=open("output/err.txt", "w"),
            )
            
            if running != 0:
                return running, i
        # Prendo dal file txt dei tempi e li sommo
        with open(f'./times/time_{exe}.txt', 'r') as f:
            count_line = 0
            for line in f:
                totalTime += float(line)
                count_line += 1
                
        times.append(round(totalTime / count_line, 2))
    return 0, 0


def main():
    
    for i in range(3):
        with open(f"times/time_{EXE_FILENAMES[i]}.txt", "w") as f:
            f.write("")
    
    configurations = [
    # (n, m, num_particles, num_threads, iters)
        (10, 10, 30, 4, 100),
        (10, 10, 30, 4, 1000),
        (100, 100, 400, 4, 100),
        (100, 100, 400, 4, 1000),
        (100, 100, 400, 4, 10000),
        (100, 100, 400, 8, 10000),
        (100, 100, 700, 4, 1000),
        (1000, 1000, 300000, 4, 1000),
        (1000, 1000, 300000, 8, 1000),
        (1000, 1000, 600000, 4, 1000),
        (1000, 1000, 600000, 8, 1000),
    ]
    #code, i = run_test(configurations, "dla_pthread")
    # if code != 0:
    #     print(f"Errore {code} durante l'esecuzione del test {i} \n {configurations[i]}")
    #     return
    # print(times)
    # with open(f"./times/history.txt", "a") as f:
    #     f.write(f"{configurations} \n {times} \n \n")
    
    
    plot.getPlot(numTests, 
                [0.0, 0.0, 0.0, 0.0, 0.05, 0.12708333333333333, 0.18214285714285713, 4.7453125, 11.33611111111111, 23.64875, 38.798863636363635],
                configurations)
    
    
if __name__ == "__main__":
    main()
    