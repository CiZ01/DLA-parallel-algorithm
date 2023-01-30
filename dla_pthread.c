#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "support_functions.c"
#include "timer.h"

int num_threads; // numero di thread
int n, m, num_particles, horizon;
int seed[2];               // seed
cell **matrix;             // matrice di punttori a celle
pthread_barrier_t barrier; // barriera per sincronizzare i thread

float coefficent; // coefficiente di aggregazione

gdImagePtr p_img; // puntatore all'immagine

void *start_DLA_parallel(void *rank);
int check_position_parallel(int n, int m, cell **matrix, particle *p, stuckedParticles *sp);
void gen_particles_parallel(int *seed, int my_num_particles, particle *my_particles_list, int n, int m);

/*
 * Genera una lista di particelle con posizione casuale.
 * La funzione riceve in input il numero di particelle da generare, il seed iniziale della simulazione, la lista di particelle e le misure della matrice.
 * La funzione ritorna un errore nel caso in cui il numero di particelle sia maggiore della dimensione della matrice.
 * La funzione ritorna un errore nel caso in cui non riesca ad allocare memoria per la posizione della particella e per lo storico dei movimenti.
 * La funzione modifica la lista di particelle.
 */
void gen_particles_parallel(int *seed, int my_num_particles, particle *my_particles_list, int n, int m)
{

    if (my_num_particles >= n * m)
    {
        perror("Troppe particelle all'interno della matrice. \n");
        exit(1);
    }

    for (int i = 0; i < my_num_particles; i++)
    {
        // allocate memory for particle position
        my_particles_list[i].current_position = (position *)malloc(sizeof(position));
        if (my_particles_list[i].current_position == NULL)
        {
            perror("Error allocating memory for current_position. \n");
            exit(1);
        }

        // genera una posizione casuale per la particella,
        // se la posizione è già occupata dal seme genera una nuova posizione
        do
        {
            my_particles_list[i].current_position->x = rand_r(&gen_rand) % m;
            my_particles_list[i].current_position->y = rand_r(&gen_rand) % n;
        } while (seed[0] == my_particles_list[i].current_position->x && seed[1] == my_particles_list[i].current_position->y);

        my_particles_list[i].dire = rand_r(&gen_rand) % 2 == 0 ? 1 : -1;
        my_particles_list[i].stuck = 0;
        my_particles_list[i].isOut = 0;
    }
}

/*
 * Controlla tutti i possibili movimenti che potrebbe fare la particella in una superficie 2D.
 * La funzione ritorna un intero che indica se la particella è rimasta bloccata o meno.
 * Se la particella si è aggregata, la funzione ritorna -1, altrimenti ritorna 0.
 * La funzione riceve in input le dimensioni della matrice, la matrice e la particella interessata.
 * La funzione modifica la matrice e la particella SOLO se la particella è rimasta bloccata e quessto
 * lo fa all'iterazione successiva per non interferire con le altre particelle.
 */
int check_position_parallel(int n, int m, cell **matrix, particle *p, stuckedParticles *sp)
{
    if (p->isOut == 1)
    {
        return 0;
    }

    int directions[] = {0, 1, 0, -1, 1, 0, -1, 0, 1, 1, 1, -1, -1, 1, -1, -1};

    for (int i = 0; i < 8; i += 2)
    {
        int near_y = p->current_position->y + directions[i];
        int near_x = p->current_position->x + directions[i + 1];
        if (near_x >= 0 && near_x < m && near_y >= 0 && near_y < n)
        {
            if (matrix[near_y][near_x].value == 1)
            {
                if(sp_append(sp, *p) != 0)
                {
                    perror("Error nell'append della stuckedParticles list. \n");
                    exit(1);
                }
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

    printf("%d.Starting DLA\n", (int)my_rank);
    
    stuckedParticles stucked_particles; // lista di particelle bloccate
    
    // inizializzo la lista delle particelle bloccate
    if (init_StuckedParticles(&stucked_particles, (int)coefficent) != 0)
    {
        perror("Error nell'inizializazione della stuckedParticles list. \n");
        exit(1);
    }
   
    // create particles
    particle *my_particles_list = (particle *)malloc(sizeof(particle) * my_num_particles);
    if (my_particles_list == NULL)
    {
        perror("Error allocating memory for particles. \n");
        exit(1);
    }
    gen_particles_parallel(seed, my_num_particles, my_particles_list, n, m);

    for (int t = 0; t < horizon + 1; t++)
    {
        // Itero per ogni iterazione tutte le particelle
        for (int i = 0; i < my_num_particles; i++)
        {
            particle *p = &my_particles_list[i];
            if (p->stuck == 0)
            {
                int isStuck = check_position_parallel(n, m, matrix, p, &stucked_particles);
                if (isStuck == 0)
                {
                    if (t < horizon)
                        move_parallel(p, n, m);
                    else if (p->isOut == 0)
                        matrix[p->current_position->y][p->current_position->x].value += 2;
                }
            }
        }
        // BARRIER
        pthread_barrier_wait(&barrier);
        while (stucked_particles.size > 0)
        {
            particle p = sp_pop(&stucked_particles);
            matrix[p.current_position->y][p.current_position->x].value = 1;
        }
        pthread_barrier_wait(&barrier);
    }

    // FINALIZE THREAD //

    // free memory
    for (int i = 0; i < my_num_particles; i++)
    {
        if (my_particles_list[i].current_position != NULL)
            free(my_particles_list[i].current_position);
    }
    if (my_particles_list != NULL)
        free(my_particles_list);

    if (sp_destroy(&stucked_particles) != 0)
    {
        perror("Error nella distruzione della stuckedParticles list. \n");
        exit(1);
    }

    printf("%ld.Finished DLA \n", my_rank);
    return NULL;
}

int main(int argc, char *argv[])
{
    double start, end, elapsed;

    get_args_parallel(argc, argv, &num_particles, &n, &m, seed, &num_threads, &horizon);

    // da calibrare
    coefficent = ((num_particles * horizon) / (n * m))*(0.2/num_threads);

    matrix = (cell **)malloc(n * sizeof(cell *)); // Alloca un array di puntatori e inizializza tutti gli elementi a 0
    if (matrix == NULL)
        perror("Error allocating memory");

    for (int i = 0; i < n; i++)
    {
        matrix[i] = (cell *)malloc(m * sizeof(cell)); // Alloca un array di interi per ogni riga e inizializza tutti gli elementi a 0
        if (matrix[i] == NULL)
            perror("Error allocating memory");
        matrix[i]->value = 0;
    }

    matrix[seed[0]][seed[1]].value = 1; // set seed

    // create threads
    long thread;
    pthread_t *thread_handles;
    pthread_barrier_init(&barrier, NULL, num_threads);

    thread_handles = (pthread_t *)malloc(num_threads * sizeof(pthread_t));

    GET_TIME(start);

    for (thread = 0; thread < num_threads; thread++)
        pthread_create(&thread_handles[thread], NULL, start_DLA_parallel, (void *)thread);

    for (thread = 0; thread < num_threads; thread++)
        pthread_join(thread_handles[thread], NULL);

    GET_TIME(end);

    elapsed = (double)(end - start);

    printf("Elapsed time: %f seconds \n", elapsed);

    FILE *elapsed_time = fopen("./times/time_dla_pthread.txt", "a");
    fprintf(elapsed_time, "%f\n", elapsed);
    fclose(elapsed_time);

    char filename[100];
    sprintf(filename, "DLA_%d_%d_%d_%d_%d.png", n, m, num_particles, num_threads, horizon);
    p_img = gdImageCreate(m, n);
    int white = gdImageColorAllocate(p_img, 255, 255, 255);
    gdImageFill(p_img, 0, 0, white);
    int black = gdImageColorAllocate(p_img, 0, 0, 0);
    int red = gdImageColorAllocate(p_img, 255, 0, 0);
    int colors[] = {black, red};

    printf("Creating image... \n");
    createImage(p_img, m, n, matrix, filename, colors);

    // -----FINALIZE----- //

    printf("freed memory: ");
    for (int i = 0; i < n; i++)
    {
        if (matrix[i] != NULL)
        {
            free(matrix[i]);                          // Libera la memoria della riga i-esima
        }
    }

    printf("matrix, mutex, ");
    if (matrix != NULL)
        free(matrix); // Libera la memoria dell'array di puntatori


    if (p_img != NULL)
        gdImageDestroy(p_img); // Libera la memoria dell'immagine
    printf("gdImage pointer, ");

    pthread_barrier_destroy(&barrier); // Distrugge la barriera
    printf("barrier, ");

    if (thread_handles != NULL)
        free(thread_handles); // Libera la lista dei threads
    printf("thread_handles \n");

    return 0;
}