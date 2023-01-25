#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include "support_functions.c"

#define ITERATIONS 10000

int num_threads;
int n, m, num_particles;
int seed[2];
cell **matrix;
unsigned int rand_seed;
pthread_barrier_t barrier;
gdImagePtr p_img;
int white;
char filename[32];

void *start_DLA_parallel(void *rank);
int check_position_parallel(int n, int m, cell **matrix, particle *p);
void gen_particles_parallel(int *seed, int my_num_particles, particle *my_particles_list, int n, int m);

/*
 * gen_particles genera una lista di particelle con posizione casuale.
 * La funzione riceve in input il numero di particelle da generare, il iniziale della simulazione, la lista di particelle e le misure della matrice.
 * La funzione ritorna un errore nel caso in cui il numero di particelle sia maggiore della dimensione della matrice.
 * La funzione ritorna un errore nel caso in cui non riesca ad allocare memoria per la posizione della particella e per lo storico dei movimenti.
 * La funzione modifica la lista di particelle.
 */
void gen_particles_parallel(int *seed, int my_num_particles, particle *my_particles_list, int n, int m)
{

    if (my_num_particles >= n * m)
    {
        perror("Too many particles for the matrix size. \n");
    }

    // generate random seed
    for (int i = 0; i < my_num_particles; i++)
    {
        // allocate memory for particle position
        my_particles_list[i].current_position = (position *)malloc(sizeof(position));
        if (my_particles_list[i].current_position == NULL)
        {
            perror("Error allocating memory for current_position. \n");
        }

        do
        {
            my_particles_list[i].current_position->x = rand_r(&rand_seed) % m;
            my_particles_list[i].current_position->y = rand_r(&rand_seed) % n;
            // check if the particle is not in the same position of the seed
        } while (seed[0] == my_particles_list[i].current_position->x && seed[1] == my_particles_list[i].current_position->y);

        my_particles_list[i].vel = rand_r(&rand_seed) % 10;
        my_particles_list[i].dire = rand_r(&rand_seed) % 2 == 0 ? 1 : -1;
        my_particles_list[i].stuck = 0;
        my_particles_list[i].isOut = 0;

        
    }
}

/*
 * check_position controlla tutti i possibili movimenti che potrebbe fare la particella in una superficie 2D.
 * La funzione ritorna un intero che indica se la particella è rimasta bloccata o meno.
 * Se la particella è rimasta bloccata, la funzione ritorna -1, altrimenti ritorna 0.
 * La funzione riceve in input le dimensioni della matrice, la matrice e la particella interessata.
 * La funzione modifica la matrice e la particella SOLO se la particella è rimasta bloccata.
 */
int check_position_parallel(int n, int m, cell **matrix, particle *p)
{
    if (p->stuck == 1)
    {
        return -1;
    }

    if (p->isOut == 1)
    {
        return 0;
    }
    //else
    if (matrix[p->current_position->y][p->current_position->x].value >= 2){
        matrix[p->current_position->y][p->current_position->x].value -= 2;
    }

    int directions[] = {0, 1, 0, -1, 1, 0, -1, 0, 1, 1, 1, -1, -1, 1, -1, -1};

    for (int i = 0; i < 8; i += 2)
    {
        int near_y = p->current_position->y + directions[i];
        int near_x = p->current_position->x + directions[i + 1];
        if (near_x >= 0 && near_x < n && near_y >= 0 && near_y < m)
        {
            if (matrix[near_y][near_x].value == 1)
            {
                pthread_mutex_lock(&matrix[p->current_position->y][p->current_position->x].mutex);
                matrix[p->current_position->y][p->current_position->x].value = 1;
                pthread_mutex_unlock(&matrix[p->current_position->y][p->current_position->x].mutex);
                p->stuck = 1;
                return -1;
            }
        }
    }
    return 0;
}

void *start_DLA_parallel(void *rank)
{
    long my_rank = (long)rank;
    int my_num_particles = num_particles / num_threads;
    if (my_rank == num_threads - 1)
    {
        my_num_particles += num_particles % num_threads;
    }

    // create particles
    particle *my_particles_list = (particle *)malloc(sizeof(particle) * my_num_particles);
    if (my_particles_list == NULL)
    {
        perror("Error allocating memory for particles. \n");
    }

    printf("%d --- %d\n", (int)my_rank, my_num_particles);
    gen_particles_parallel(seed, my_num_particles, my_particles_list, n, m);

    printf("%d.Starting DLA\n", (int)my_rank);

    for (int t = 0; t < ITERATIONS; t++)
    {
        // Itero per particelle per ogni iterazione
        for (int i = 0; i < my_num_particles; i++)
        {
            particle *p = &my_particles_list[i];
            if (p->stuck == 0)
            {
                int isStuck = check_position_parallel(n, m, matrix, p);
                if (isStuck == 0)
                {
                    move_pthread(p, matrix, n, m);
                }
            }
        }
        // BARRIER
        pthread_barrier_wait(&barrier);
        if ((int)my_rank == 0)
        {
            write_matrix_cell(n, m, matrix);
            sprintf(filename, "imgs/frames/frame_%d.bmp", t);
            //time_t start_img = time(NULL);
            createImage(p_img, m, n, matrix, filename);
            gdImageFilledRectangle(p_img, 0, 0, m, n, white);
            //time_t end_img = time(NULL);
            //printf("%d. Image created in %ld seconds\n", (int)my_rank, end_img - start_img);
        }
        // tutti si devono fermare qua mentre aspettano il thread 0
        pthread_barrier_wait(&barrier);
        // BARRIER
    }

    // FINALIZE //

    // free memory
    for (int i = 0; i < my_num_particles; i++)
    {
        if (my_particles_list[i].current_position != NULL)
            free(my_particles_list[i].current_position);
    }
    if (my_particles_list != NULL)
        free(my_particles_list);
    printf("%ld.Finished DLA \n", my_rank);
    return NULL;
}

int main(int argc, char *argv[])
{

    get_args_parallel(argv, &num_particles, &n, &m, seed, &num_threads);

    printf("num_threads: %d \n", num_threads);

    matrix = (cell **)malloc(n * sizeof(cell *)); // Alloca un array di puntatori e inizializza tutti gli elementi a 0
    if (matrix == NULL)
        perror("Error allocating memory");

    for (int i = 0; i < n; i++)
    {
        matrix[i] = (cell *)malloc(m * sizeof(cell)); // Alloca un array di interi per ogni riga e inizializza tutti gli elementi a 0
        if (matrix[i] == NULL)
            perror("Error allocating memory");
        if (pthread_mutex_init(&matrix[i]->mutex, NULL))
            perror("Error initializing mutex");
        matrix[i]->value = 0;
    }

    matrix[seed[0]][seed[1]].value = 1; // set seed

    // create threads
    long thread;
    pthread_t *thread_handles;
    pthread_barrier_init(&barrier, NULL, num_threads);

    thread_handles = (pthread_t *)malloc(num_threads * sizeof(pthread_t));

    rand_seed = (unsigned int)856;

    p_img = gdImageCreate(m, n);
    white = gdImageColorAllocate(p_img, 255, 255, 255);

    time_t start = time(NULL);

    for (thread = 0; thread < num_threads; thread++)
        pthread_create(&thread_handles[thread], NULL, start_DLA_parallel, (void *)thread);

    for (thread = 0; thread < num_threads; thread++)
        pthread_join(thread_handles[thread], NULL);

    time_t end = time(NULL);

    int status = system("convert -delay 50 imgs/frames/*.bmp animation.gif");
    if (status == -1)
        perror("Error creating gif");

    printf("Elapsed time: %f seconds \n", (double)((end - start)));

    // -----FINALIZE----- //

    printf("freed memory: ");
    for (int i = 0; i < n; i++)
    {
        if (matrix[i] != NULL)
        {
            free(matrix[i]);                          // Libera la memoria della riga i-esima
            pthread_mutex_destroy(&matrix[i]->mutex); // Distrugge il mutex
        }
    }

    printf("matrix, mutex, ");
    if (matrix != NULL)
        free(matrix); // Libera la memoria dell'array di puntatori

    if (p_img != NULL)
        gdImageDestroy(p_img);
    printf("gdImage pointer, ");

    pthread_barrier_destroy(&barrier);
    printf("barrier, ");

    if (thread_handles != NULL)
        free(thread_handles);
    printf("thread_handles \n");

    return 0;
}