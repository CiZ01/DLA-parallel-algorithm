#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <omp.h>
#include "support_functions.c"

gdImagePtr img;

int thread_count; // numero di thread
float coefficent; // coefficiente di aggregazione

int check_position(int n, int m, int **matrix, particle *p, stuckedParticles *sp);


/*
 * check_position controlla tutti i possibili movimenti che potrebbe fare la particella in una superficie 2D.
 * La funzione ritorna un intero che indica se la particella è rimasta bloccata o meno.
 * Se la particella è rimasta bloccata, la funzione ritorna -1, altrimenti ritorna 0.
 * La funzione riceve in input le dimensioni della matrice, la matrice e la particella interessata.
 * La funzione modifica la matrice e la particella SOLO se la particella è rimasta bloccata.
 */
int check_position(int n, int m, int **matrix, particle *p, stuckedParticles *sp)
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
            if (matrix[near_y][near_x] == 1)
            {
                if(sp_append(sp, p) != 0)
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
    int i = 0;
    #pragma omp parallel for num_threads(thread_count) shared(gen_rand, particles_list)
    for (i = 0; i < num_particles; i++)
    {
        // allocate memory for particle position
        particles_list[i].current_position = malloc(sizeof(position));
        if (particles_list[i].current_position == NULL)
        {
            perror("Error allocating memory for current_position. \n");
        }
        do
        {
            particles_list[i].current_position->x = rand_r(&gen_rand) % m;
            particles_list[i].current_position->y = rand_r(&gen_rand) % n;
            // check if the particle is not in the same position of the seed
        } while (seed[0] == particles_list[i].current_position->x && seed[1] == particles_list[i].current_position->y);
        particles_list[i].dire = rand_r(&gen_rand) % 2 == 0 ? 1 : -1;
        particles_list[i].stuck = 0;
        particles_list[i].isOut = 0;
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
               int **matrix, int horizon)
{
    printf("Starting DLA\n");

    stuckedParticles sp; // lista di particelle bloccate

    // inizializzo la lista delle particelle bloccate
    if (init_StuckedParticles(&sp, (int)coefficent) != 0)
    {
        perror("Error nell'inizializazione della stuckedParticles list. \n");
        exit(1);
    }

    for (int t = 0; t < horizon + 1; t++)
    {
        // Itero per particelle per ogni iterazione
        int i;
        #pragma omp parallel for num_threads(thread_count) shared(particles_list, matrix)
        for (i = 0; i < num_particles; i++)
        {
            particle *p = &particles_list[i];
            if (p->stuck <= 0)
            {
                int isStuck = check_position(n, m, matrix, p, &sp);
                if (isStuck == 0)
                {
                    if (t < horizon)
                        move_parallel(p, n, m);
                    else if (p->isOut == 0)
                        matrix[p->current_position->y][p->current_position->x] += 2;
                }
            }
        }
        #pragma omp barrier
        int j;
        #pragma omp parallel for num_threads(thread_count) shared(particles_list, matrix, sp)
        for (j = 0; j < sp.size; j++)
        {
            particle p;
            sp_pop(&sp, &p);
            matrix[p.current_position->y][p.current_position->x] = 1;
        }
        #pragma omp barrier
    }
    printf("Finished DLA\n");
}

int main(int argc, char *argv[])
{

    int n, m;          // matrix dimensions
    int seed[2];       // seed position
    int num_particles; // number of particles
    int horizon;      // horizon
    int **matrix;     //matri

    get_args_parallel(argc, argv, &num_particles, &n, &m, seed, &thread_count, &horizon);

    coefficent = (num_particles * horizon) / (n * m);

    printf("seed %d, %d\n", seed[0], seed[1]);

    matrix = (int **)calloc(n, sizeof(int *)); // Alloca un array di puntatori e inizializza tutti gli elementi a 0
    if (matrix == NULL)
        perror("Error allocating memory");

    for (int i = 0; i < n; i++)
    {
        matrix[i] = (int *)calloc(m, sizeof(int)); // Alloca un array di interi per ogni riga e inizializza tutti gli elementi a 0
        if (matrix[i] == NULL)
            perror("Error allocatingx memory");
    }

    matrix[seed[0]][seed[1]] = 1; // set seed

    particle *particles_list = (particle *)malloc(sizeof(particle) * num_particles);
    if (particles_list == NULL)
        perror("Error allocating memory");

    // create particles
    gen_particles(seed, num_particles, particles_list, n, m);

    // start DLA
    double start = omp_get_wtime();
    start_DLA(num_particles, particles_list, n, m, matrix, horizon);
    double end = omp_get_wtime();

    double elapsed = (double)(end - start);
    
    printf("il tempo impiegato per il DLA è: %f \n", elapsed);

    FILE *elapsed_time = fopen("./times/time_dla_openmp.txt", "a");
    fprintf(elapsed_time, "%f\n", elapsed);
    fclose(elapsed_time);
    // -----SAVE DATA----- //

    img = gdImageCreate(m, n);
    int white = gdImageColorAllocate(img, 255, 255, 255);
    gdImageFilledRectangle(img, 0, 0, m, n, white);
    
    int black = gdImageColorAllocate(img, 0, 0, 0);
    int red = gdImageColorAllocate(img, 255, 0, 0);

    int colors[] = {black, red};

    //filename a caso da sistemare
    createImage_intMatrix(img, m, n, matrix, colors, "DLA_openmp.jpg");
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
    }
    printf("particles's path and current_position, ");

    if (particles_list != NULL)
        free(particles_list);
    printf("particles_list \n");

    return 0;
}