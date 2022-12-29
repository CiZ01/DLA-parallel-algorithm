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


def set_seed():
    '''
    Aggiunge il seed alla matrice.
    Da questo seed si svilupperà il cristallo.
    
    Il seed viene posizionato in modo pseudo casuale.
    
    return: tuple(i,j) che rappresenta la posizione del seed nella matrice.
    '''
    r.seed(time.time())
    return r.randint(0, n - 1), r.randint(0, m - 1)


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

    #particles_list = particle_generate(num_particles)

    #print(f"input:{seed}|{particles_list}")

    arg = f"{n},{m}|{seed[0]},{seed[1]}|{num_particles}".split("|")
    cmd_running = [f"./{c_file[:-2]}"] + arg
    #print(cmd_running)

    start = time.time()
    try:
        running = subprocess.check_call(
            cmd_running,
            stdout=open("out.txt", "w"),
            stderr=open("err.txt", "w"),
        )
    except subprocess.CalledProcessError as e:
        running = e.returncode
    end = time.time()

    print(f"Return code : {running}")

    global total_time
    total_time += end - start

    return seed


def get_output_matrix():
    '''
    Legge il file di output e restituisce la matrice.
    '''
    matrix = []
    with open(OUTPUT_FILE, "r") as f:
        for line in f:
            matrix += [line.split(" ")[:-1]]
    if len(matrix) == 0:
        with open("errors.txt", "a") as f:
            f.write("Matrix is empty\n")
            return []
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


def make_img(paths, seed):
    matrix = get_output_matrix()

    n, m = len(matrix), len(matrix[0])

    img = [[(255, 255, 255) for i in range(m)] for j in range(n)]

    img[seed[0]][seed[1]] = (0, 0, 0)

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

            if (y < n and x < m) and (y >= 0 and x >= 0):
                if (next_y, next_x) == (0, 0):
                    img[y][x] = (0, 0, 0)
                    blank[y][x] = (0, 0, 0)
                else:
                    img[y][x] = (255, 0, 0)

        images.save(img, f"imgs/matrix{i//2}.png")
        img = blank.copy()
    return


def make_matrix():
    tmpmatrix = get_output_matrix()
    if len(tmpmatrix) == 0:
        print("--------- Matrix is empty --------- \n")
        return []

    n, m = len(tmpmatrix), len(tmpmatrix[0])

    print(f"n: {n} m: {m}")

    matrix = [[(255, 255, 255) for i in range(m)] for j in range(n)]

    for i in range(n):
        for j in range(m):
            if tmpmatrix[i][j] == "1":
                matrix[i][j] = (0, 0, 0)
            elif tmpmatrix[i][j] == "2":
                matrix[i][j] = (255, 0, 0)
            else:
                matrix[i][j] = (255, 255, 255)
    return matrix


def make_gif():
    frames = [Image.open(image) for image in glob.glob(f"imgs/matrix*.png")]
    frame_one = frames[0]
    frame_one.save("paths.gif",
                   format="GIF",
                   append_images=frames,
                   save_all=True,
                   duration=300,
                   loop=0)


def main():
    for i in range(100):
        print("iterazione: ", i)
        seed = execute_c_program()
        print(seed)
        #paths = get_output_paths()
        #make_img(paths, seed)  # genera le immagini per ogni iterazione
        matrix = make_matrix()
        if len(matrix) != 0:
            images.save(matrix, f"./imgs/matrix_f{i}.png")
        else:
            break

    #make_gif()
    print(f"tempo totale: {total_time}")
    return


if __name__ == '__main__':
    main()
