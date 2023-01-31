#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <omp.h>
#include "support_functions.c" // File con le funzioni comuni

gdImagePtr img; // oggetto immagine

int thread_count;  // numero di thread
float coefficient; // coefficiente di aggregazione

void gen_particles_openMP(int *seed, int num_particles, particle *particles_list, int n, int m);      // Generatore di particelle
void start_DLA(int num_particles, particle *particles_list, int n, int m, int **matrix, int horizon); // Funzione DLA

/*
 * Genera una lista di particelle con posizione casuale.
 * La funzione modifica la lista di particelle.
 * @param seed: posizione del seme
 * @param num_particles: numero di particelle da generare
 * @param particles_list: lista di particelle
 * @param n: numero di righe della matrice
 * @param m: numero di colonne della matrice
 */
void gen_particles_openMP(int *seed, int num_particles, particle *particles_list, int n, int m)
{
    // Per evitare una saturazione della matrice poniamo un limite alle particelle da generare
    if (num_particles >= n * m)
    {
        perror("Troppe particelle rispetto alla grandezza della matrice. \n");
        exit(3);
    }
    int i = 0;
#pragma omp parallel for num_threads(thread_count) shared(gen_rand, particles_list)
    for (i = 0; i < num_particles; i++)
    {
        // Allochiamo memoria per la posizione delle particelle
        particles_list[i].current_position = malloc(sizeof(position));
        if (particles_list[i].current_position == NULL)
        {
            perror("Errore nell'allocazione di memoria per la current_position. \n");
            exit(1);
        }
        do
        {
            particles_list[i].current_position->x = rand_r(&gen_rand) % m;
            particles_list[i].current_position->y = rand_r(&gen_rand) % n;
            // Controllo che la particella non venga generata sul seed
        } while (seed[0] == particles_list[i].current_position->x && seed[1] == particles_list[i].current_position->y);
        particles_list[i].dire = 0;
        particles_list[i].stuck = 0;
        particles_list[i].isOut = 0;
    }
}

/*
 * start_DLA simula l'algoritmo DLA.
 * La simulazione del movimento consiste:
 *  - in un ciclo che scorre tutte le particelle
 *  - per ogni particella viene chiamata la funzione check_position che controlla se la particella è in prossimità di un cristallo,
 *    in caso affermativo blocca la particella e la aggiunge ad una lista. Altrimenti la particella si muove.
 *  - nel caso in cui la particella non sia rimasta bloccata, viene chiamata la funzione move che si occupa di muoverla.
 *  - finito il ciclo sulle particelle, viene letta tutta la lista di particelle stucke, e viene aggiornata la matrice
 *
 * L'algoritmo termina al raggiungimento di un valore t.
 * @param num_particles: il numero di particelle
 * @param particle_list: la lista di particelle
 * @param n: alteza matrice
 * @param m: lunghezza matrice
 * @param horizon: numero di tick
 */
void start_DLA(int num_particles,
               particle *particles_list,
               int n, int m,
               int **matrix, int horizon)
{
    printf("Starting DLA\n");

    stuckedParticles sp; // Lista di particelle bloccate

    // Inizializzo la lista delle particelle bloccate
    if (init_StuckedParticles(&sp, (int)coefficient) != 0)
    {
        perror("Errore nell'inizializazione della stuckedParticles list. \n");
        exit(1);
    }
    // Itero per ogni tick
    for (int t = 0; t < horizon + 1; t++)
    {
        // Itero sulle particelle
        int i;
#pragma omp parallel for num_threads(thread_count) shared(particles_list, matrix)
        for (i = 0; i < num_particles; i++)
        {
            particle *p = &particles_list[i];
            // Se la particella non è stuck
            if (p->stuck == 0)
            {
                // Controllo la posizione
                int isStuck = check_position(n, m, matrix, p, &sp);
                if (isStuck == 0)
                {
                    // if and else per fare un ulitmo check se qualche particella si è stuckata
                    if (t < horizon)
                        move_parallel(p, n, m);
                    else if (p->isOut == 0)
                        // Coloro la matrice per tracciare le particelle
                        matrix[p->current_position->y][p->current_position->x] += 2;
                }
            }
        }
// Barrier per evitare che determinati thread passino all'iterazione successiva prima di altri
#pragma omp barrier
        int j;
// Svuoto l'array delle particelle stucked
#pragma omp parallel for num_threads(thread_count) shared(particles_list, matrix, sp)
        for (j = 0; j < sp.size; j++)
        {
            // Rimuovo la particella dall'array
            particle p = sp_pop(&sp);
            if (p.current_position != NULL)
            {
                // Modifico la matrice con un operazzione atomica per evitare concorrenza
                #pragma omp atomic write
                matrix[p.current_position->y][p.current_position->x] = 1;
            }
        }
#pragma omp barrier
    }

    if (sp_destroy(&sp) != 0)
    {
        perror("Error nella distruzione della stuckedParticles list. \n");
        exit(1);
    }

    printf("Finished DLA\n");
}

int main(int argc, char *argv[])
{

    int n, m;          // Dimensioni matrice
    int seed[2];       // Posizione seed
    int num_particles; // numero di particelle
    int horizon;       // Orizonte o tempo di esecuzione
    int **matrix;      // Dichiaro la matrice

    // Prendo gli argomenti dalla riga di comando
    get_args_parallel(argc, argv, &num_particles, &n, &m, seed, &thread_count, &horizon);

    // Calcolo il coefficente della realloc per l'array delle particelle stucked
    coefficient = (float)(((float)num_particles / (float)(n * m) * 100) * FACTOR) / thread_count;
    printf("coefficient: %f\n", coefficient);
    printf("seed %d, %d\n", seed[0], seed[1]);

    // Alloco un array di puntatori e inizializza tutti gli elementi a 0
    matrix = (int **)malloc(n * sizeof(int *));
    if (matrix == NULL)
    {
        perror("Errore nell'allocazione di memoria per la matrice");
        exit(1);
    }
    for (int i = 0; i < n; i++)
    {
        matrix[i] = (int *)calloc(m, sizeof(int));
        if (matrix[i] == NULL)
        {
            perror("Errore nell'allocazione di memoria per la matrice");
            exit(1);
        }
    }

    // Metto il seed nella matrice
    matrix[seed[1]][seed[0]] = 1;

    // Alloco la lista di tutte le particelle
    particle *particles_list = (particle *)malloc(sizeof(particle) * num_particles);
    if (particles_list == NULL)
    {
        perror("Errore nell'allocazione di memoria per la lista di particella");
        exit(1);
    }
    // Genero tutte le particelle e le inserisco nell'array
    gen_particles_openMP(seed, num_particles, particles_list, n, m);

    // start DLA e calcolo il tempo di esecuzione per i test
    double start = omp_get_wtime();
    start_DLA(num_particles, particles_list, n, m, matrix, horizon);
    double end = omp_get_wtime();

    double elapsed = (double)(end - start);

    printf("il tempo impiegato per il DLA è: %f \n", elapsed);
    // Salvo i tempi di esecuzione
    FILE *elapsed_time = fopen("./times/time_dla_openmp.txt", "a");
    fprintf(elapsed_time, "%f\n", elapsed);
    fclose(elapsed_time);

    // Creo l'immagine della matrice
    img = gdImageCreate(m, n);
    int white = gdImageColorAllocate(img, 255, 255, 255);
    gdImageFilledRectangle(img, 0, 0, m, n, white);

    int black = gdImageColorAllocate(img, 0, 0, 0);
    int red = gdImageColorAllocate(img, 255, 0, 0);
    int colors[] = {black, red};

    createImage(img, m, n, matrix, colors, "DLA_openmp.jpg");

    // Libero la memoria
    printf("Memoria liberata per: ");
    for (int i = 0; i < n; i++)
    {
        if (matrix[i] != NULL)
            free(matrix[i]); // Libero la memoria della riga i-esima
    }

    printf("la matrice, ");
    if (matrix != NULL)
        free(matrix); // Libero la memoria dell'array di puntatori

    for (int i = 0; i < num_particles; i++)
    {
        if (particles_list[i].current_position != NULL)
            free(particles_list[i].current_position); // Libero la memoria della current position
    }
    printf("current position, ");

    if (particles_list != NULL)
        free(particles_list);
    printf("particles_list \n"); // Libero la memoria dalla lista di particelle

    if (img != NULL)
        gdImageDestroy(img); // Libera la memoria dell'immagine
    printf("gdImage pointer");

    return 0;
}