#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "support_functions.c"
#include "timer.h"

void gen_particles(int *seed, int num_particles, particle *particles_list, int n, int m);
void start_DLA(int num_particles, particle *particles_list, int n, int m, int **matrix, int horizon, stuckedParticles sp);
void move(particle *p, int n, int m);
void get_args(int argc, char *argv[], int *num_particles, int *n, int *m, int *seed, int *horizon);

/*
 * Recupera tutti gli argomenti passati in input al programma e li setta alle opportune variabili.
 * Prende come parametri le variabili da settare:
 * @param argc numero di argomenti passati in input
 * @param argv array di argomenti passati in input
 * @param num_particles numero di particelle
 * @param n numero di righe della matrice
 * @param m numero di colonne della matrice
 * @param seed posizione iniziale della particella
 * @param horizon numero di iterazioni massime
 */
void get_args(int argc, char *argv[], int *num_particles, int *n, int *m, int *seed, int *horizon)
{
    // recupero le dimensioni della matrice
    char *token = strtok(argv[1], ",");
    *n = (int)atoi(token);
    token = strtok(NULL, ",");
    *m = (int)atoi(token);

    // recupero il numero di particelle
    *num_particles = (int)atoi(argv[2]);

    // genero random il seed - DEFAULT
    seed[0] = (int)rand_r(&gen_rand) % *n;
    seed[1] = (int)rand_r(&gen_rand) % *m;

    // setto il numero di iterazioni massime - DEFAULT
    *horizon = HORIZON;

    int opt;
    while ((opt = getopt(argc, argv, "-t:-s:")) != -1)
    {
        switch (opt)
        {
        case 't':
            // recupero il numero di iterazioni massime - OPTION
            *horizon = atoi(optarg);
            break;
        case 's':
            // recupero la posizione iniziale del seed - OPTION
            token = strtok(optarg, ",");
            seed[0] = (int)atoi(token);
            token = strtok(NULL, ",");
            seed[1] = (int)atoi(token);
            break;
        case '?':
            printf("Usage: %s [-n num_threads] [-t horizon] [-s seed] n,m num_particles \n", argv[0]);
            exit(2);
        }
    }
}

/*
 * Genera una lista di particelle con posizione casuale.
 * La funzione modifica la lista di particelle.
 * @param seed: posizione del seme
 * @param num_particles: numero di particelle da generare
 * @param particles_list: lista di particelle
 * @param n: numero di righe della matrice
 * @param m: numero di colonne della matrice
 */
void gen_particles(int *seed, int num_particles, particle *particles_list, int n, int m)
{

    if (num_particles >= n * m)
    {
        perror("Troppe particelle all'interno della matrice. \n");
        exit(3);
    }

    for (int i = 0; i < num_particles; i++)
    {
        // allocate memory for particle position
        particles_list[i].current_position = (position *)malloc(sizeof(position));
        if (particles_list[i].current_position == NULL)
        {
            perror("Errore durante l'allocazione della lista di particelle. \n");
            exit(1);
        }

        // setto la posizione della particella, se è uguale al seed la ricreo
        do
        {
            particles_list[i].current_position->x = rand() % m;
            particles_list[i].current_position->y = rand() % n;
        } while (seed[0] == particles_list[i].current_position->x && seed[1] == particles_list[i].current_position->y);

        particles_list[i].dire = 0;
        particles_list[i].stuck = 0;
        particles_list[i].isOut = 0;
    }
}

/*
 * Muove la particella in una direzione pseudocasuale.
 * Se la particelle esce fuori dalla matrice viene settata la variabile isOut a 1.
 * Se la particella non esce fuori dalla matrice viene settata la variabile isOut a 0.
 * @param p puntatore alla particella da muovere
 * @param n numero di righe della matrice
 * @param m numero di colonne della matrice
 */
void move(particle *p, int n, int m)
{

    // calcolo la nuova posoizione della particella
    p->dire = rand() % 2 == 0 ? 1 : -1;
    p->current_position->x += rand() % 2 * p->dire;

    p->dire = rand() % 2 == 0 ? 1 : -1;
    p->current_position->y += rand() % 2 * p->dire;

    // controllo se la particella è uscita dalla matrice o no
    if (!(p->current_position->x >= 0 && p->current_position->x < m && p->current_position->y >= 0 && p->current_position->y < n))
    {
        p->isOut = 1;
        return;
    }
    else
    {
        p->isOut = 0;
    }
}

/*
 * La funzione inizia la simulazione DLA.
 * La simulazione consiste in:
 *  - un ciclo che simula il tempo, ogni tick è un'iterazione della simulazione
 *  - per ogni particella viene chiamata la funzione check_position che controlla se la particella è in prossimità di un cristallo,
 *    in caso affermativo setta un flag. Altrimenti la particella si muove.
 *  - nel caso in cui la particella non sia rimasta bloccata, viene chiamata la funzione move che si occupa di muoverla.
 *  - se la particella è rimasta bloccata viene aggiornata la matrice al termine di ogni tick
 * Al termine di ogni tick viene aggiornata la matrice.
 * @param num_particles numero di particelle
 * @param particles_list lista di particelle
 * @param n numero di righe della matrice
 * @param m numero di colonne della matrice
 * @param matrix matrice di interi
 * @param horizon numero di iterazioni massime
 * @param sp lista di particelle bloccate
 */
void start_DLA(int num_particles,
               particle *particles_list,
               int n, int m,
               int **matrix, int horizon, stuckedParticles sp)
{
    printf("Starting DLA\n");

    // tick della simulazione
    for (int t = 0; t < horizon + 1; t++)
    {
        // itero su tutte le particelle
        for (int i = 0; i < num_particles; i++)
        {
            particle *p = &particles_list[i];
            if (p->stuck <= 0)
            {
                // controllo se la particella si aggrega o no
                int isStuck = check_position(n, m, matrix, p, &sp);
                if (isStuck == 0)
                {
                    if (t < horizon)
                        // muovo la particella
                        move(p, n, m);
                    else if (p->isOut == 0)
                        // se sono all'ultima iterazione e la particella non è uscita dalla matrice la aggiungo alla matrice per il render finale
                        matrix[p->current_position->y][p->current_position->x] += 2;
                }
            }
        }
        // aggiorno la matrice
        // per ogni particella stucked scrivo 1 sulla matrice nella posizione della particella
        while (sp.size > 0)
        {
            particle p = sp_pop(&sp);
            matrix[p.current_position->y][p.current_position->x] = 1;
        }
    }
    printf("Finished DLA\n");
}

int main(int argc, char *argv[])
{

    int n, m;          // matrix dimensions
    int seed[2];       // seed position
    int num_particles; // numero di particelle
    int horizon;       // tempo di simulazione
    int **matrix;
    stuckedParticles sp; // lista di particelle bloccate

    double start, end, elapsed;

    get_args(argc, argv, &num_particles, &n, &m, seed, &horizon);

    printf("Initial seed position: %d, %d\n", seed[0], seed[1]);

    matrix = (int **)malloc(n * sizeof(int *)); // Alloca un array di puntatori e inizializza tutti gli elementi a 0
    if (matrix == NULL)
    {
        perror("Errore nell'allocazione della matrice. \n");
        exit(1);
    }
    for (int i = 0; i < n; i++)
    {
        matrix[i] = (int *)calloc(m, sizeof(int)); // Alloca un array di interi per ogni riga e inizializza tutti gli elementi a 0
        if (matrix[i] == NULL)
        {
            perror("Errore nell'allocazione della matrice. \n");
            exit(1);
        }
    }

    matrix[seed[1]][seed[0]] = 1; // scrivo il seed sulla matrice

    // prendo la percentuale di quanto è satura
    // la matrice e la moltiplico per un fattore costante.
    float coefficient = (float)(((float)num_particles / (float)(n * m) * 100) * FACTOR);

    if (init_StuckedParticles(&sp, (int)coefficient) != 0)
    {
        perror("Errore nell'allocazione della lista di particelle bloccate. \n");
        exit(1);
    }
    particle *particles_list = (particle *)malloc(sizeof(particle) * num_particles);
    if (particles_list == NULL)
    {
        perror("Errore durante l'allocazione della lista di particelle. \n");
        exit(1);
    }

    srand(seed_rand); // setto il seed per la random

    // genero le particelle
    gen_particles(seed, num_particles, particles_list, n, m);

    GET_TIME(start);

    // start DLA
    start_DLA(num_particles, particles_list, n, m, matrix, horizon, sp);

    GET_TIME(end);

    elapsed = (double)(end - start);
    printf("Elapsed time: %f seconds \n", elapsed);

    // ----- TIME ----- //
    FILE *elapsed_time = fopen("./times/time_dla_single_thread.txt", "a");
    fprintf(elapsed_time, "%f\n", elapsed);
    fclose(elapsed_time);

    // ----- RENDER ----- //
    gdImagePtr img = gdImageCreate(m, n);
    int white = gdImageColorAllocate(img, 255, 255, 255);
    gdImageFilledRectangle(img, 0, 0, m, n, white);
    int colors[2];
    colors[0] = gdImageColorAllocate(img, 0, 0, 0);   // black
    colors[1] = gdImageColorAllocate(img, 255, 0, 0); // red
    createImage(img, m, n, matrix, colors, "single_thread_render.jpg");

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

    printf("destroy image pointer, ");
    gdImageDestroy(img);

    if (sp_destroy(&sp) != 0)
    {
        perror("Error nella distruzione della stuckedParticles list. \n");
        exit(1);
    }

    for (int i = 0; i < num_particles; i++)
    {
        if (particles_list[i].current_position != NULL)
            free(particles_list[i].current_position);
    }
    printf("current_position, ");

    if (particles_list != NULL)
        free(particles_list);
    printf("particles_list \n");

    return 0;
}