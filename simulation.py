import random as r
import time


class DLA:

    def __init__(self, n: int, m: int, numParticles: int):
        '''
        Inizializza la matrice e il seed.
        Setta il seed sulla matrice e setta il numero di particelle.
        '''
        self.n = n
        self.m = m
        self.final_matrix = [[(255, 255, 255) for i in range(m)]
                             for j in range(n)]
        self.seed = self.set_seed()
        self.numParticles = numParticles
        self.final_matrix[self.seed[0]][self.seed[1]] = (0, 0, 0)

        self.paths = ()
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
        tmpMatrix = read_matrix_from_file(filename)

        for i in range(self.n):
            for j in range(self.m):
                if tmpMatrix[i][j] == "1":
                    self.final_matrix[i][j] = (0, 0, 0)

    def set_paths_from_file(self, filename: str):
        '''
        Legge il file di output che ha scritto il programma C e genera il risultato finale.
        '''
        tmpPaths = read_paths_from_file(filename)
        self.paths = tuple(zip(tmpPaths[::2], tmpPaths[1::2]))

    def set_seed(self) -> tuple:
        '''
        Aggiunge il seed alla matrice.
        Da questo seed si svilupperà il cristallo.
        
        Il seed viene posizionato in modo pseudo casuale.
        
        Ritorna tuple(i,j) che rappresenta la posizione del seed nella matrice.
        '''
        r.seed(time.time())
        return r.randint(0, self.n - 1), r.randint(0, self.m - 1)


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


def read_paths_from_file(filename: str) -> tuple:
    '''
    Legge il file di output e restituisce il path di tutte le particelle.
    '''

    paths = []
    with open(filename, "r") as f:
        for line in f:
            paths += [int(s) for s in line.split(",")[:-1]]
    if len(paths) == 0:
        raise Exception("Paths is empty")
    return paths
