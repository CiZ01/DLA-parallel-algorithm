import random as r
import time
import multiprocessing as mp
import os
import numpy as np


class DLA:

    def __init__(self, n: int, m: int, numParticles: int, numTests : int, numThreads : int):
        '''
        Inizializza la matrice e il seed.
        Setta il seed sulla matrice e setta il numero di particelle.
        '''
        self.n = n
        self.m = m
        self.final_matrix = [[(255, 255, 255) for _ in range(m)] for _ in range(n)]
        self.seed = self.set_seed()
        self.final_matrix[self.seed[0]][self.seed[1]] = (0, 0, 0)
        

        self.numParticles = numParticles

        self.paths = ()

        self.numThreads = numThreads
        self.isParallel = True if self.numThreads > 1 else False
            
        self.numTests = numTests
        
        self.elapsedTimes = set()
        self.avgElapsedTime = 0
        pass

    def set_numParticles(self, numParticles: int):
        '''
        Setta il numero di particelle.
        '''
        self.numParticles = numParticles

    def set_matrix_from_file(self, filename: str):
        '''
        Legge il file di output che ha scritto il programma C e genera il risultato finale.
        '''

        tmpMatrix = read_matrix_from_file_parallel(
            filename, self.n +
            1) if self.isParallel else read_matrix_from_file(filename)

        for i in range(self.n):
            for j in range(self.m):
                if tmpMatrix[i][j] == "1":
                    self.final_matrix[i][j] = (0, 0, 0)

    def set_paths_from_file(self, filename: str):
        '''
        Legge il file di output che ha scritto il programma C e genera il risultato finale.
        '''
        tmpPaths = read_paths_from_file_parallel(
            filename, self.numParticles
        ) if self.isParallel else read_paths_from_file(filename)

        paths = []
        for row in tmpPaths:
            paths += [list(zip(row[::2], row[1::2]))]

        self.paths = paths

    def set_seed(self) -> tuple:
        '''
        Aggiunge il seed alla matrice.
        Da questo seed si svilupperà il cristallo.
        
        Il seed viene posizionato in modo pseudo casuale.
        
        Ritorna tuple(i,j) che rappresenta la posizione del seed nella matrice.
        '''
        r.seed(time.time())
        return r.randint(0, self.n - 1), r.randint(0, self.m - 1)

    def set_parallel(self, parallel):
        '''
        Setta il flag per la modalità parallela.
        '''
        self.isParallel = parallel


def read_matrix_from_file(filename: str) -> list:
    '''
    Legge il file di output e restituisce la matrice.
    '''
    matrix = []
    with open(filename, "r") as f:
        for line in f:
            matrix += [line.split(" ")[:-1]]
    if len(matrix) == 0:
        raise Exception("Matrix is empty")
    return matrix


def read_matrix_from_file_parallel(filename: str, numLines: int) -> list:
    '''
    Legge il file di output e restituisce la matrice.
    '''

    matrix = read_file_parallel(filename, numLines)

    return matrix


def read_paths_from_file(filename: str) -> list:
    '''
    Legge il file di output e restituisce il path di tutte le particelle.
    '''

    paths = []
    with open(filename, "r") as f:
        for line in f:
            paths += [[int(s) for s in line.split(",")[:-1]]]
    if len(paths) == 0:
        raise Exception("Paths is empty")
    return paths


def read_paths_from_file_parallel(filename: str, numLines) -> list:
    '''
    Legge il file dei paths in parallelo e restituisce il path di tutte le particelle.
    I paths sono salvati in una lista di liste.
    '''

    tmpPaths = read_file_parallel(filename, numLines)

    if len(tmpPaths) == 0:
        raise Exception("Paths is empty")

    paths = np.array([])
    for row in tmpPaths:
        paths = np.append(paths, [list(zip(row[::2], row[1::2]))])

    print(paths)
    return paths


def worker(lines):
    result = {}
    for i, line in enumerate(lines):
        v = line.split()
        result[i] = v
    return result


def read_file_parallel(filename: str, numlines: int) -> list:

    # configurable options.  different values may work better.
    numthreads = 4

    lines = open(filename).readlines()

    # create the process pool
    pool = mp.Pool(processes=numthreads)

    # map the list of lines into a list of result dicts
    result_list = pool.map(worker,
                           (lines[line:line + numlines]
                            for line in range(0, len(lines), numlines)))
    # reduce the result dicts into a single dict
    result = {}
    list(map(result.update, result_list))

    return np.array(list(result.values()))


if '__main__' == __name__:
    sim = DLA(5, 5, 5, True)

    sim.set_matrix_from_file("matrix.txt")
    sim.set_paths_from_file("paths.txt")

    print(sim.final_matrix)
    print(sim.paths)
    pass