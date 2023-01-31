#!/bin/python3

import subprocess


EXE_FILENAMES = ("dla_single_thread", "dla_pthread", "dla_openmp")
numTests = 5
times = []

def run_test(configurations: list) -> tuple:
    for config in configurations:
        totalTime = 0
        n, m, num_particles, num_threads, iters, exe = config

        if num_threads > 0 and exe != "dla_single_thread":
            args = f"{n},{m}|{num_particles}|-n{num_threads}|-s{n//2},{m//2}|-t{iters}".split(
                "|")
        else:
            args = f"{n},{m}|{num_particles}|-s{n//2},{m//2}|-t{iters}".split(
                "|")

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
        # (n, m, num_particles, num_threads, iters, filename)
        (1000, 1000, 400000, 0, 10000, 'dla_single_thread'),
        (1000, 1000, 400000, 4, 10000, 'dla_pthread'),
        (1000, 1000, 400000, 4, 10000, 'dla_openmp'),
        (1000, 1000, 400000, 8, 10000, 'dla_pthread'),
        (1000, 1000, 400000, 8, 10000, 'dla_openmp'),
        (1000, 1000, 400000, 16, 10000, 'dla_pthread'),
        (1000, 1000, 400000, 16, 10000, 'dla_openmp')
    ]

    code, i = run_test(configurations)
    if code != 0:
        print(
            f"Errore {code} durante l'esecuzione del test {i} \n {configurations[i]}"
        )
        return
    print(times)
    with open(f"./times/history.txt", "a") as f:
        f.write(f"{configurations} \n {times} \n \n")




if __name__ == "__main__":
    main()
