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
import sys
import subprocess
import os
import time
import glob
from PIL import Image

RANDOM_SEED = time.time()  # 1629740000.0
OUTPUT_FILE = "matrix.txt"
NUM_THREADS = 4
NUM_PARTICLES = 10

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


def set_seed():
    '''
    Aggiunge il seed alla matrice.
    Da questo seed si svilupperà il cristallo.
    
    Il seed viene posizionato in modo pseudo casuale.
    
    return: tuple(i,j) che rappresenta la posizione del seed nella matrice.
    '''
    r.seed(RANDOM_SEED)
    return r.randint(0, m - 1), r.randint(0, n - 1)


def particle_generate(num_particles):
    return ', '.join({
        f"{r.randint(0, n-1)}, {r.randint(0, m-1)}, {r.randint(0, 10)}"
        for i in range(num_particles)
    })


def execute_c_program():
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

    
    DA IMPLEMENTARE:    teoricamente può anche aspettare termini e prendersi il return. 
                        per ora l'idea è che il programma C scriva su file.
    '''
    #cmd = f"gcc -g -Wall -o {c_file[:-2]} {c_file}"
    # compiling
    #compiling = subprocess.run(cmd.split(" "), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    #print_out(compiling)
    seed = set_seed()
    # running

    particles_list = particle_generate(num_particles)
    for i in range(0, num_particles, 3):
        if particles_list[i] == seed[0] and particles_list[i + 1] == seed[1]:
            particles_list = particles_list[0:i] + particles_list[i + 3:]

    print(f"input:{seed}|{particles_list}")

    arg = f"{n},{m}|{seed[0]},{seed[1]}|{particles_list}|{num_particles}".split(
        "|")
    cmd_running = [f"./{c_file[:-2]}"] + arg
    #print(cmd_running)
    running = subprocess.run(cmd_running,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)
    print(running.stderr.decode('utf-8', 'replace'))
    print(running.stdout.decode('utf-8'))
    pass


def get_output_matrix():
    '''
    Legge il file di output e restituisce la matrice.
    '''
    matrix = []
    with open(OUTPUT_FILE, "r") as f:
        for line in f:
            matrix += [line.split(" ")[:-1]]
    return matrix


def get_output_paths():
    '''
    Legge il file di output e restituisce il path di tutte le particelle.
    '''

    paths = []
    with open('paths.txt', "r") as f:
        for line in f:
            paths += [line.split(",")[:-1]]
    return paths


def make_img(paths):
    matrix = get_output_matrix()
    n, m = len(matrix), len(matrix[0])

    img = Image.new('RGB', (n, m))
    data = img.load()

    for i in range(m):
        for j in range(n):
            data[j, i] = (255, 255, 255)

    blank = img.copy()

    # 1,2,4,5,6,7

    for i in range(0, len(paths[0]), 2):
        for j in range(0, len(paths)):
            y = int(paths[j][i])
            x = int(paths[j][i + 1])
            #print(x)
            #print(y)

            if i + 2 < len(paths[0]):
                next_y = int(paths[j][i + 2])
                next_x = int(paths[j][i + 3])
            else:
                next_y = -1
                next_x = -1

            if (y < n and x < m) and (y > 0 and x > 0):
                if (next_y, next_x) == (0, 0):
                    data[x, y] = (0, 0, 0)
                else:
                    data[x, y] = (255, 0, 0)

        img.save(f"imgs/matrix{i-1}.png")
        img = blank.copy()
        data = img.load()
    return


def make_img_matrix() -> Image:
    matrix = get_output_matrix()
    n, m = len(matrix), len(matrix[0])

    img = Image.new('RGB', (n, m))
    data = img.load()

    #print(matrix)
    for i in range(n):
        for j in range(m):
            if matrix[i][j] == "1":
                data[j, i] = (0, 0, 0)
            elif matrix[i][j] == "2":
                data[j, i] = (255, 0, 0)
            else:
                data[j, i] = (255, 255, 255)
    return img


def make_gif():
    frames = [Image.open(image) for image in glob.glob(f"imgs/*.png")]
    frame_one = frames[0]
    frame_one.save("paths.gif",
                   format="GIF",
                   append_images=frames,
                   save_all=True,
                   duration=1000,
                   loop=0)


def main():
    for i in range(1):
        execute_c_program()
        paths = get_output_paths()
        make_img(paths)  # genera le immagini per ogni iterazione
        img = make_img_matrix()
        img.save(f"./imgs/matrix_f.png")

    make_gif()
    return


if __name__ == '__main__':
    main()
