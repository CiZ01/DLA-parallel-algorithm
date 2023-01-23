#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "support_functions.c"

#define ITERATIONS 1000


// DA IMPLEMENTARE
//  typedef int (*start_dla)(int  n, int m, int matrix[n][m], particle *p);
//  typedef void (*move)(particle p);
//  typedef int (*check_position)(int  n, int m, int matrix[n][m], particle *p);

void gen_particles(int *seed, int num_particles, particle *particles_list, int n, int m);
void start_DLA(int num_particles, particle *particles_list, int n, int m, int **matrix);
int check_position(int n, int m, int **matrix, particle *p);


/*
 * check_position controlla tutti i possibili movimenti che potrebbe fare la particella in una superficie 2D.
 * La funzione ritorna un intero che indica se la particella è rimasta bloccata o meno.
 * Se la particella è rimasta bloccata, la funzione ritorna -1, altrimenti ritorna 0.
 * La funzione riceve in input le dimensioni della matrice, la matrice e la particella interessata.
 * La funzione modifica la matrice e la particella SOLO se la particella è rimasta bloccata.
 */
int check_position(int n, int m, int **matrix, particle *p)
{
    if (p->stuck == 1)
    {
        return -1;
    }

    int directions[] = {0, 1, 0, -1, 1, 0, -1, 0, 1, 1, 1, -1, -1, 1, -1, -1};

    for (int i = 0; i < 8; i += 2)
    {
        int near_y = p->current_position->y + directions[i];
        int near_x = p->current_position->x + directions[i + 1];
        if (near_x >= 0 && near_x < n && near_y >= 0 && near_y < m)
        {
            if (matrix[near_y][near_x] == 1)
            {
                if (p->current_position->x >= 0 && p->current_position->x < n && p->current_position->y >= 0 && p->current_position->y < m)
                {
                    matrix[p->current_position->y][p->current_position->x] = 1;
                    p->stuck = 1;
                    // p->path = (position *)realloc(p->path, sizeof(position) * (p->size_path + 1));
                    // if (p->path == NULL)
                    // {
                    //     perror("Error reallocating memory");
                    // }
                    return -1;
                }
            }
        }
    }
    return 0;
}



/*
 * gen_particles genera una lista di particelle con posizione casuale.
 * La funzione riceve in input il numero di particelle da generare, il iniziale della simulazione, la lista di particelle e le misure della matrice.
 * La funzione ritorna un errore nel caso in cui il numero di particelle sia maggiore della dimensione della matrice.
 * La funzione ritorna un errore nel caso in cui non riesca ad allocare memoria per la posizione della particella e per lo storico dei movimenti.
 * La funzione modifica la lista di particelle.
 */
void gen_particles(int *seed, int num_particles, particle *particles_list, int n, int m)
{

    if (num_particles >= n * m)
    {
        perror("Too many particles for the matrix size. \n");
    }

    for (int i = 0; i < num_particles; i++)
    {
        // allocate memory for particle position
        particles_list[i].current_position =(position*)malloc(sizeof(position));
        if (particles_list[i].current_position == NULL)
        {
            perror("Error allocating memory for current_position. \n");
        }
        do
        {
            particles_list[i].current_position->x = rand() % m;
            particles_list[i].current_position->y = rand() % n;
            // check if the particle is not in the same position of the seed
        } while (seed[0] == particles_list[i].current_position->x && seed[1] == particles_list[i].current_position->y);

        particles_list[i].vel = rand() % 10;
        particles_list[i].dire = rand() % 2 == 0 ? 1 : -1;
        particles_list[i].stuck = 0;

        // allocate memory for particle path
        // particles_list[i].path = malloc(sizeof(position) * ITERATIONS);
        // if (particles_list[i].path == NULL)
        // {
        //     perror("Error allocating memory for paths. \n");
        // }
        // // set the first position of the path
        // particles_list[i].path[0] = *particles_list[i].current_position;
        // particles_list[i].size_path = 1;
    }
}

/*
 * start_DLA simula l'algoritmo DLA.
 * Printa la matrice una prima volta e poi simula il movimento di ogni particella per un valore t.
 * La simulazione del movimento consiste:
 *  - in un ciclo che scorre tutte le particelle
 *  - per ogni particella viene chiamata la funzione check_position che controlla se la particella è in prossimità di un cristallo,
 *    in caso affermativo blocca la particella e da quel momento inizia a far parte del cristallo. Altrimenti la particella si muove.
 *  - nel caso in cui la particella non sia rimasta bloccata, viene chiamata la funzione move che si occupa di muoverla.
 *
 * L'algoritmo termina al raggiungimento di un valore t.
 * La funzione riceve in input il numero di particelle, la lista di particelle, le dimensioni della matrice e la matrice.
 * La funzione ritorna 0 se l'esecuzione è andata a buon fine, altrimenti ritorna 1.
 *
 */
void start_DLA(int num_particles,
               particle *particles_list,
               int n, int m,
               int **matrix)
{
    printf("Starting DLA\n");
    for (int t = 1; t < ITERATIONS; t++)
    {
        // Itero per particelle per ogni iterazione
        for (int i = 0; i < num_particles; i++)
        {
            particle *p = &particles_list[i];
            if (p->stuck == 0)
            {
                int isStuck = check_position(n, m, matrix, p);
                if (isStuck == 0)
                {
                    move(p);
                    // p->path[t] = *p->current_position;
                    // p->size_path++;
                }
            }

        }
    }
    printf("Finished DLA\n");
}

int main(int argc, char *argv[])
{

    int n, m;          // matrix dimensions
    int seed[2];       // seed position
    int num_particles; // number of particles

    get_args(argv, &num_particles, &n, &m, seed);
    // printf("num_particles: %d, n: %d, m: %d, seed: %d, %d\n", num_particles, n, m, seed[0], seed[1]);
    // fflush(stdout);
    // num_particles = 50;
    // n = 10;
    // m = 10;
    // seed[0] = 50;
    // seed[1] = 50;

    printf("seed %d, %d\n", seed[0], seed[1]);

    int **matrix;
    matrix = (int **)calloc(n, sizeof(int *)); // Alloca un array di puntatori e inizializza tutti gli elementi a 0
    if (matrix == NULL)
        perror("Error allocating memory");

    for (int i = 0; i < n; i++)
    {
        matrix[i] = (int *)calloc(m, sizeof(int)); // Alloca un array di interi per ogni riga e inizializza tutti gli elementi a 0
        if (matrix[i] == NULL)
            perror("Error allocating memory");
    }

    matrix[seed[0]][seed[1]] = 1; // set seed

    particle *particles_list = (particle *)malloc(sizeof(particle) * num_particles);
    if (particles_list == NULL)
        perror("Error allocating memory");

    srand(856); // set seed for random
    // create particles
    gen_particles(seed, num_particles, particles_list, n, m);

    time_t start = time(NULL);
    // start DLA
    start_DLA(num_particles, particles_list, n, m, matrix);

    time_t end = time(NULL);
    printf("time: %ld\n", (end - start));
    // -----SAVE DATA----- //
    // save matrix
    //write_matrix(n, m, matrix);

    // save paths
    //write_paths(num_particles, particles_list);
    gdImagePtr img = gdImageCreate(m, n); 
    createImage_intMatrix(img, n, m, matrix);
    saveImage(img, "matrix_single_thread.png");

    // -----FINALIZE----- //

    printf("freed memory: ");
    for (int i = 0; i < n; i++)
    {
        if (matrix[i] != NULL)
            free(matrix[i]); // Libera la memoria della riga i-esima
    }

    printf("matrix, ");
    if (matrix != NULL)
        free(matrix); // Libera la memoria dell'array di puntatori
    
  
    for (int i = 0; i < num_particles; i++)
    {
        if (particles_list[i].current_position != NULL)
            free(particles_list[i].current_position);
        // if (particles_list[i].path != NULL)
        //     free(particles_list[i].path);
    }
    printf("particles's path and current_position, ");

    if (particles_list != NULL)
        free(particles_list);
    printf("particles_list \n");

    return 0;
}