#! /bin/python3
'''
Questo script produce dei test per il progetto 5 di Sistemi Embedded e Multicore 2022/2023.
Più che dei test produce dei valori da passare al programma scritto in C.
In seguito il programma costruirà i suoi dati di partenza da questi valori.

Il progetto consiste nello sviluppare un algoritmo multi processo in MPI e una versione implementata in Cuda
per lo studio del Diffusion Limited Aggregation (DLA).
Più precisamente l'algoritmo deve studiare la crescita di un cristallo posizionato in una matrice 2D, 
le particelle vengono posizionate casualmente e si muovono in modo casuale, seguono un Moto Browniano.
'''

import random as r
import simulation as sim
import sys
import subprocess
import os
import time
import glob
import images
from PIL import Image

RANDOM_SEED = time.time()  # 1629740000.0
OUTPUT_FILE = "matrix.txt"
NUM_THREADS = 4
NUM_PARTICLES = 10

total_time = 0

C_FILE = "dla_single_thread.c"


def print_out(out: subprocess.CompletedProcess):
    text = out.stderr.decode('utf-8', 'replace')
    text = text.replace("warning", "\033[1;35;40mwarning\033[0m ")
    text = text.replace("error", "\033[1;31;40merror\033[0m ")
    print(text)
    return


'''
È possibile passare i seguenti parametri:
>*  n,m: dimensioni della matrice, rispettivamente numero di righe e colonne.
>*  num_particles: numero totale di particelle.
>   num_threads: numero di thread da usare per il calcolo parallelo. Se non specificato viene usato il valor NUM_THREADS.
>   c_file: eseguibile C da usare per il calcolo parallelo. Se non specificato viene usato `DA DEFINIRE`.
>   output_filename: file di output. Se non specificato viene usato `output.txt`.

I paramatri evidenziati con * sono obbligatori.
Per ora questi parametri non sono implementati come opzioni.
'''
for arg in sys.argv:
    n = int(
        sys.argv[1].split(',')
        [0])  # immagino che il primo argomento venga passato nella forma n,m
    m = int(sys.argv[1].split(',')[1])

    num_particles = int(sys.argv[2]) if len(sys.argv) > 2 else NUM_PARTICLES

    num_threads = int(sys.argv[3]) if len(sys.argv) > 3 else NUM_THREADS

    c_file = sys.argv[4] if len(sys.argv) > 4 else C_FILE
    output_file = sys.argv[5] if len(sys.argv) > 5 else OUTPUT_FILE

simulation = sim.DLA(n, m, num_particles)


def execute_c_program() -> int:
    '''
    Compila il programma C.
    Passa i parametri al programma e lo esegue.
    
    I parametri da passare sono:
    >*  n,m: dimensioni della matrice, rispettivamente numero di righe e colonne.
    >*  particles_list: lista di particelle da posizionare nella matrice. 
        Le particelle sono dichiarate nel seguente modo: (i , j , v).
        - i: riga
        - j: colonna
        - v: velocità
    >*  num_particles: numero totale di particelle.
    >   num_threads: numero di thread da usare per il calcolo parallelo. Se non specificato viene usato il valor NUM_THREADS.

    '''
    # recupero il seed dalla simulazione
    seed = simulation.seed

    # Formatto i parametri da passare al programma C.
    args = f"{n},{m}|{seed[0]},{seed[1]}|{num_particles}".split("|")
    cmd_running = [f"./{c_file[:-2]}"] + args

    start = time.time()
    try:
        # Eseguo il programma C.
        # Se il programma termina con un errore viene lanciata un'eccezione.
        running = subprocess.check_call(
            cmd_running,
            stdout=open("out.txt", "w"),
            stderr=open("err.txt", "w"),
        )
    except subprocess.CalledProcessError as e:
        running = e.returncode
    end = time.time()

    global total_time
    total_time += end - start

    return running


def main():

    err = execute_c_program()
    if err != 0:
        print(f"Errore nell'esecuzione del programma C. Error code: {err}")
        return err

    return 0


if __name__ == '__main__':
    main()
