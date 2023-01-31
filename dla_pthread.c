#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include "support_functions.c"
#include "timer.h"

int num_threads; // numero di thread
int n, m, num_particles, horizon;
int seed[2];               // seed
int **matrix;              // matrice di interi
pthread_barrier_t barrier; // barriera per sincronizzare i thread

float coefficient; // coefficiente di aggregazione

gdImagePtr p_img; // puntatore all'immagine

void *start_DLA_parallel(void *rank);
int check_position_parallel(int n, int m, int **matrix, particle *p, stuckedParticles *sp);
void gen_particles_parallel(int *seed, int my_num_particles, particle *my_particles_list, int n, int m);

/*
 * Genera una lista di particelle con posizione casuale.
 * La funzione modifica la lista di particelle.
 * @param seed: posizione del seme
 * @param num_particles: numero di particelle da generare
 * @param particles_list: lista di particelle
 * @param n: numero di righe della matrice
 * @param m: numero di colonne della matrice
 */
void gen_particles_parallel(int *seed, int my_num_particles, particle *my_particles_list, int n, int m)
{

    if (my_num_particles >= n * m)
    {
        perror("Troppe particelle all'interno della matrice. \n");
        exit(3);
    }

    for (int i = 0; i < my_num_particles; i++)
    {
        // allocate memory for particle position
        my_particles_list[i].current_position = (position *)malloc(sizeof(position));
        if (my_particles_list[i].current_position == NULL)
        {
            perror("Errore nell'allocazione della lista di particelle \n");
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
 * La funzione inizia la simulazione DLA.
 * Ogni thread esegue la simulazione e viene sincronizzato con gli altri thread con una barriera al termine di ogni tick.
 * All'inizio della simulazione viene inizializzata la lista delle particelle stucked, e viene generata una lista di particelle casuali.
 * La funzione inizia la simulazione DLA.
 * La simulazione consiste in:
 *  - un ciclo che simula il tempo, ogni tick è un'iterazione della simulazione
 *  - per ogni particella viene chiamata la funzione check_position che controlla se la particella è in prossimità di un cristallo,
 *    in caso affermativo setta un flag. Altrimenti la particella si muove.
 *  - nel caso in cui la particella non sia rimasta bloccata, viene chiamata la funzione move che si occupa di muoverla.
 *  - se la particella è rimasta bloccata viene aggiornata la matrice al termine di ogni tick
 * Al termine di ogni tick viene aggiornata la matrice.
 * @param rank: rank del thread
 */
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
    if (init_StuckedParticles(&stucked_particles, (int)coefficient) != 0)
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
                int isStuck = check_position(n, m, matrix, p, &stucked_particles);
                if (isStuck == 0)
                {
                    if (t < horizon)
                        move_parallel(p, n, m);
                    else if (p->isOut == 0)
                        matrix[p->current_position->y][p->current_position->x] += 2;
                }
            }
        }
        // BARRIER
        pthread_barrier_wait(&barrier);
        while (stucked_particles.size > 0)
        {
            particle p = sp_pop(&stucked_particles);
            matrix[p.current_position->y][p.current_position->x] = 1;
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

    // recupero i parametri da riga di comando
    get_args_parallel(argc, argv, &num_particles, &n, &m, seed, &num_threads, &horizon);

    // prendo la percentuale di quanto è satura la matrice e la moltiplico per un fattore costante.
    // dal momento che istanzio una lista di particelle stucked per ogni thread, divido il risultato per il numero di threads
    coefficient = (float)(((float)num_particles / (float)(n * m) * 100) * FACTOR) / (num_threads*0.5);

    // Alloca un array di puntatori a interi per ogni riga
    matrix = (int **)malloc(n * sizeof(int *));
    if (matrix == NULL)
        perror("Error allocating memory");

    for (int i = 0; i < n; i++)
    {
        matrix[i] = (int *)calloc(m, sizeof(int)); // Alloca un array di interi per ogni riga e inizializza tutti gli elementi a 0
        if (matrix[i] == NULL)
            perror("Error allocating memory");
    }

    matrix[seed[1]][seed[0]] = 1; // scrivo il seed sulla matrice

    // create threads
    long thread;
    pthread_t *thread_handles;
    pthread_barrier_init(&barrier, NULL, num_threads);

    thread_handles = (pthread_t *)malloc(num_threads * sizeof(pthread_t));

    GET_TIME(start);

    // creo i threads
    for (thread = 0; thread < num_threads; thread++)
        pthread_create(&thread_handles[thread], NULL, start_DLA_parallel, (void *)thread);

    // aspetto che i threads terminino
    for (thread = 0; thread < num_threads; thread++)
        pthread_join(thread_handles[thread], NULL);

    GET_TIME(end);

    elapsed = (double)(end - start);

    printf("Elapsed time: %f seconds \n", elapsed);

    //-----TIME------//

    FILE *elapsed_time = fopen("./times/time_dla_pthread.txt", "a");
    fprintf(elapsed_time, "%f\n", elapsed);
    fclose(elapsed_time);

    //---------------//

    // -----IMAGE----- //
    char filename[100];
    sprintf(filename, "DLA_%d_%d_%d_%d_%d.png", n, m, num_particles, num_threads, horizon);
    p_img = gdImageCreate(m, n);

    int white = gdImageColorAllocate(p_img, 255, 255, 255);
    gdImageFill(p_img, 0, 0, white);

    int black = gdImageColorAllocate(p_img, 0, 0, 0);
    int red = gdImageColorAllocate(p_img, 255, 0, 0);
    int colors[] = {black, red};

    printf("Creating image... \n");
    createImage(p_img, m, n, matrix, colors, filename);

    // -----FINALIZE----- //

    printf("freed memory: ");
    for (int i = 0; i < n; i++)
    {
        if (matrix[i] != NULL)
        {
            free(matrix[i]); // Libera la memoria della riga i-esima
        }
    }

    printf("matrix, ");
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