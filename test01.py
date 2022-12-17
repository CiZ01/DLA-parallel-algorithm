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

import random
import sys
import subprocess
import os

RANDOM_SEED = 42
OUTPUT_FILE = "output.txt"
NUM_THREADS = 4

C_EXE = "hello_world.c"

os.environ['SYSTEMD_COLORS'] = '1'

'''
È possibile passare i seguenti parametri:
>*  n,m: dimensioni della matrice, rispettivamente numero di righe e colonne. 
>  num_threads: numero di thread da usare per il calcolo parallelo. Se non specificato viene usato il valor NUM_THREADS.
>   c_exe: eseguibile C da usare per il calcolo parallelo. Se non specificato viene usato `DA DEFINIRE`.
>   output_filename: file di output. Se non specificato viene usato `output.txt`.

I paramatri evidenziati con * sono obbligatori.
Per ora questi parametri non sono implementati come opzioni.
'''
for arg in sys.argv:
    n = int(sys.argv[1].split(',')[0]) # immagino che il primo argomento venga passato nella forma n,m
    m = int(sys.argv[1].split(',')[1])
    
    num_threads = int(sys.argv[2]) if len(sys.argv) > 2 else NUM_THREADS
    
    c_exe = sys.argv[3] if len(sys.argv) > 3 else C_EXE
    output_file = sys.argv[4] if len(sys.argv) > 4 else OUTPUT_FILE
    
    
    
def set_seed():
    '''
    Aggiunge il seed alla matrice.
    Da questo seed si svilupperà il cristallo.
    
    Il seed viene posizionato in modo pseudo casuale.
    
    return: tuple(i,j) che rappresenta la posizione del seed nella matrice.
    '''
    random.seed(RANDOM_SEED)
    return (random.randint(0, n-1), random.randint(0, m-1))


def execute_c_program():
    '''
    Compila il programma C.
    Passa i parametri al programma e lo esegue.
    
    DA IMPLEMENTARE:    teoricamente può anche aspettare termini e prendersi il return. 
                        per ora l'idea è che il programma C scriva su file.
    '''
    compiling = subprocess.run(["gcc",'-g','-Wall', c_exe], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    print(compiling.stdout.decode('utf-8','replace'))
    print(compiling.stderr.decode('utf-8','replace'))
    compiling = subprocess.run(["./a.out"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    print(compiling.stdout.decode('utf-8','replace'))
    pass
    
def main():
    execute_c_program()
    return

if __name__ == '__main__':
    main()
    
    