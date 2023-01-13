import os
import imageio
import multiprocessing as mp
import numpy as np


def create_row(row_index):
    return [(255, 255, 255) for _ in range(m)]


m = 100000


def gen_matrix_parallel(n, m):

    num_cores = mp.cpu_count()

    with mp.Pool(num_cores) as p:
        matrix = p.map(create_row, range(n))

    # convertiamo la lista di righe in una matrice NumPy
    matrix = np.array(matrix)

    print(matrix)


if "__main__" == __name__:
    #gen_matrix_parallel(100000, 100000)
    m = np.empty((100000, 100000), dtype=np.uint8)
    pass
