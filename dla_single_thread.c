#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ITERATIONS 100

typedef struct
{
    int x; // position on x axis
    int y; // position on y axis
} position;

typedef struct
{
    position *current_position;

    int vel;  // velocity
    int dire; // direction

    int stuck;      // 0 = not stuck, 1 = stuck
    position *path; // history path of the particle
} particle;

// DA IMPLEMENTARE
//  typedef int (*start_dla)(int  n, int m, int matrix[n][m], particle *p);
//  typedef void (*move)(particle p);
//  typedef int (*check_position)(int  n, int m, int matrix[n][m], particle *p);

void get_args(char *argv[], unsigned int *num_particles, unsigned int *n, unsigned int *m, unsigned int *seed);
void print_matrix(unsigned int n, unsigned int m, int **matrix);
void move(particle *part, unsigned int n, unsigned int m, int **matrix);
void gen_particles(unsigned int num_particles, particle *particles_list, unsigned int n, unsigned int m);
int start_DLA(unsigned int num_particles, particle *particles_list, unsigned int n, unsigned int m, int **matrix);
int check_position(unsigned int n, unsigned int m, int **matrix, particle *p);

void get_args(char *argv[], unsigned int *num_particles, unsigned int *n, unsigned int *m, unsigned int *seed)
{
    // get matrix dimensions
    char *sizes = argv[1];
    char *token = strtok(sizes, ",");
    *n = (unsigned int)atoi(token);
    token = strtok(NULL, ",");
    *m = (unsigned int)atoi(token);

    // get seed position
    char *seed_pos = argv[2];
    token = strtok(seed_pos, ",");
    seed[0] = (unsigned int)atoi(token);
    token = strtok(NULL, ",");
    seed[1] = (unsigned int)atoi(token);

    // get number of particles
    *num_particles = (unsigned int)atoi(argv[3]);
}

/*
 * check_position controlla tutti i possibili movimenti che potrebbe fare la particella in una superficie 2D.
 * La funzione ritorna un intero che indica se la particella è rimasta bloccata o meno.
 * Se la particella è rimasta bloccata, la funzione ritorna -1, altrimenti ritorna 0.
 * La funzione riceve in input le dimensioni della matrice, la matrice e la particella interessata.
 * La funzione modifica la matrice e la particella SOLO se la particella è rimasta bloccata.
 */
int check_position(unsigned int n, unsigned int m, int **matrix, particle *p)

{
    if (p->stuck == 1)
    {
        return -1;
    }

    int directions[] = {0, 1, 0, -1, 1, 0, -1, 0, 1, 1, 1, -1, -1, 1, -1, -1};

    for (unsigned int i = 0; i < 8; i += 2)
    {
        int near_y = p->current_position->y + directions[i];
        int near_x = p->current_position->x + directions[i + 1];
        if (near_x > 0 && near_x < n && near_y > 0 && near_y < m)
        {
            if (matrix[near_y][near_x] == 1)
            {
                matrix[p->current_position->y][p->current_position->x] = 1;
                p->stuck = 1;
                return -1;
            }
        }
    }
    return 0;
}

/*
 * move muove la particella in una direzione pseudocasuale.
 * La funzione riceve in inpute la particella, le dimensioni della matrice e la matrice.
 * La funzione modifica la matrice e la particella a ogni chiamata simulando il movimento della particella.
 * Quest'ultima funzionalità penso sia solo a scopo di TEST.
 * La funzione non ritorna nulla.
 */
void move(particle *p, unsigned int n, unsigned int m, int **matrix)
{

    // move particle
    p->dire = rand() % 2 == 0 ? 1 : -1;
    p->current_position->x += rand() % 2 * p->dire;

    p->dire = rand() % 2 == 0 ? 1 : -1;
    p->current_position->y += rand() % 2 * p->dire;
}

/*
 * gen_particles genera una lista di particelle a partire da una stringa.
 * La funzione riceve in input il numero di particelle da generare, la lista di particelle e la stringa.
 * La stringa deve essere formattata nel seguente modo: "i,j,v,i,j,v,i,j,v,..." dove i e j sono le coordinate della particella e v è la velocità.
 * La funzione ritorna 0 se la generazione è andata a buon fine, altrimenti ritorna 1.
 * La funzione modifica la lista di particelle.
 */
void gen_particles(unsigned int num_particles, particle *particles_list, unsigned int n, unsigned int m)
{

    if (num_particles >= n * m)
    {
        printf("Too many particles for the matrix size. \n");
        exit(1);
    }

    // get data from particle_arg
    srand(time(NULL));
    for (unsigned int i = 0; i < num_particles; i++)
    {
        particles_list[i].current_position = malloc(sizeof(position));
        particles_list[i].current_position->x = rand() % m;
        particles_list[i].current_position->y = rand() % n;
        particles_list[i].vel = rand() % 10;
        particles_list[i].dire = rand() % 2 == 0 ? 1 : -1;
        particles_list[i].stuck = 0;
        particles_list[i].path = malloc(sizeof(position) * ITERATIONS);
    }
}

/*
 * print_matrix stampa la matrice.
 * La funzione riceve in input le dimensioni della matrice e la matrice.
 * La funzione non ritorna nulla.
 */
void print_matrix(unsigned int n, unsigned int m, int **matrix)
{
    unsigned int a, b;
    for (a = 0; a < n; a++)
    {
        for (b = 0; b < m; b++)
        {
            printf("%d ", matrix[a][b]);
        }
        printf("\n");
    }
    printf("-----------------------------------------------------\n");
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
int start_DLA(unsigned int num_particles,
              particle *particles_list,
              unsigned int n, unsigned int m,
              int **matrix)
{
    printf("Starting DLA\n");
    unsigned int t;
    srand(time(NULL));
    for (t = 1; t < ITERATIONS; t++)
    {
        unsigned int i;
        for (i = 0; i < num_particles; i++)
        {
            particle *p = &particles_list[i];
            if (p->stuck == 0)
            {
                int isStuck = check_position(n, m, matrix, p);
                if (isStuck == 0)
                {
                    move(p, n, m, matrix);
                }
                p->path[t] = *p->current_position;
            }
        }
    }
    printf("Finished DLA\n");
    return 0;
}

int main(int argc, char *argv[])
{

    unsigned int n, m;          // matrix dimensions
    unsigned int seed[2];       // seed position
    unsigned int num_particles; // number of particles

    get_args(argv, &num_particles, &n, &m, seed);
    printf("num_particles: %d, n: %d, m: %d, seed: %d, %d\n", num_particles, n, m, seed[0], seed[1]);
    // num_particles = 500000;
    // n = 1000;
    // m = 1000;
    // seed[0] = 50;
    // seed[1] = 50;

    int **matrix;
    matrix = calloc(n, sizeof(int *)); // Alloca un array di puntatori e inizializza tutti gli elementi a 0

    for (int i = 0; i < n; i++)
    {
        matrix[i] = calloc(m, sizeof(int)); // Alloca un array di interi per ogni riga e inizializza tutti gli elementi a 0
    }

    matrix[seed[0]][seed[1]] = 1; // set seed

    particle *particles_list = malloc(sizeof(particle) * num_particles);

    // create particles and check for errors
    gen_particles(num_particles, particles_list, n, m);

    // print_matrix(n, m, matrix);
    // fflush(stdout);
    // start DLA
    start_DLA(num_particles, particles_list, n, m, matrix);

    // print_matrix(n, m, matrix);
    // fflush(stdout);
    FILE *fptr;

    // use appropriate location if you are using MacOS or Linux
    fptr = fopen("matrix.txt", "w");

    if (fptr == NULL)
    {
        printf("Error!");
        exit(1);
    }

    for (unsigned int i = 0; i < n; i++)
    {
        for (unsigned int j = 0; j < m; j++)
        {
            fprintf(fptr, "%d ", matrix[i][j]);
        }
        fprintf(fptr, "\n");
    }

    // close file
    fclose(fptr);

    FILE *fptr2;
    fptr2 = fopen("paths.txt", "w");

    if (fptr2 == NULL)
    {
        printf("Error!");
        exit(1);
    }

    for (unsigned int i = 0; i < num_particles; i++)
    {
        particle *p = &particles_list[i];
        for (unsigned int j = 0; j < ITERATIONS; j++)
        {
            fprintf(fptr2, "%d,%d,", p->path[j].y, p->path[j].x);
        }
        fprintf(fptr2, "\n");
    }

    fclose(fptr2);

    // free matrix
    for (int i = 0; i < n; i++)
    {
        free(matrix[i]); // Libera la memoria della riga i-esima
    }

    free(matrix);

    // free memory
    for (unsigned int i = 0; i < num_particles; i++)
    {
        particle *p = &particles_list[i];
        free(p->current_position);
        free(p->path);
    }
    free(particles_list);

    return 0;
}