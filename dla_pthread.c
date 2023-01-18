#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include "support_functions.c"

#define ITERATIONS 1000

typedef struct
{
    int value;
    pthread_mutex_t mutex;
} cell;

int num_threads;
int n,m,num_particles;
int seed[2];
cell** matrix;
unsigned int rand_seed;




void *start_DLA_parallel(void* rank);
int check_position_parallel(int n, int m, cell **matrix, particle *p);
void gen_particles_parallel(int *seed, int my_num_particles, particle *my_particles_list, int n, int m);
void get_args_pthreads(char *argv[], int *num_particles, int *n, int *m, int *seed, int *num_threads);
void move_parallel(particle *p);

void move_parallel(particle *p)
{
    
    // move particle
    p->dire = rand_r(&rand_seed) % 2 == 0 ? 1 : -1;
    p->current_position->x += rand_r(&rand_seed) % 2 * p->dire;

    p->dire = rand_r(&rand_seed) % 2 == 0 ? 1 : -1;
    p->current_position->y += rand_r(&rand_seed) % 2 * p->dire;
}
/*
 * Recupera tutti gli argomenti passati in input al programma e li setta alle opportune variabili.
 * In caso di mancato argomento il programma termina per un segmentation fault.
 */
void get_args_pthreads(char *argv[], int *num_particles, int *n, int *m, int *seed, int *num_threads)
{
    char *sizes = argv[1];
    char *token = strtok(sizes, ",");
    *n = (int)atoi(token);
    token = strtok(NULL, ",");
    *m = (int)atoi(token);

    // get seed position
    char *seed_pos = argv[2];
    token = strtok(seed_pos, ",");
    seed[0] = (int)atoi(token);
    token = strtok(NULL, ",");
    seed[1] = (int)atoi(token);

    // get number of particles
    *num_particles = (int)atoi(argv[3]);

    *num_threads = (int)atoi(argv[4]);
}


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
        my_particles_list[i].current_position = (position* )malloc(sizeof(position));
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

        // allocate memory for particle path
        //my_particles_list[i].path = (position* )malloc(sizeof(position) * ITERATIONS);
        //if (my_particles_list[i].path == NULL)
        //{
        //    perror("Error allocating memory for paths. \n");
        //}
        // set the first position of the path
        //my_particles_list[i].path[0] = *my_particles_list[i].current_position;
        //my_particles_list[i].size_path = 1;
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

    int directions[] = {0, 1, 0, -1, 1, 0, -1, 0, 1, 1, 1, -1, -1, 1, -1, -1};

    for (int i = 0; i < 8; i += 2)
    {
        int near_y = p->current_position->y + directions[i];
        int near_x = p->current_position->x + directions[i + 1];
        if (near_x >= 0 && near_x < n && near_y >= 0 && near_y < m)
        {
            if (matrix[near_y][near_x].value == 1)
            {
                if (p->current_position->x >= 0 && p->current_position->x < n && p->current_position->y >= 0 && p->current_position->y < m)
                {
                    pthread_mutex_lock(&matrix[p->current_position->y][p->current_position->x].mutex);
                    matrix[p->current_position->y][p->current_position->x].value = 1;
                    pthread_mutex_unlock(&matrix[p->current_position->y][p->current_position->x].mutex);
                    p->stuck = 1;
                    //p->path = (position *)realloc(p->path, sizeof(position) * (p->size_path + 1));
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

void *start_DLA_parallel(void *rank)
{
    long my_rank = (long)rank;
    int my_num_particles = num_particles / num_threads;
    if (my_rank == num_threads - 1)
    {
        my_num_particles += num_particles % num_threads;
    }



    // create particles

    particle*  my_particles_list = (particle *)malloc(sizeof(particle) * my_num_particles);
    if (my_particles_list == NULL)
    {
        perror("Error allocating memory for particles. \n");
    }
    printf("%d --- %d\n", (int)my_rank, my_num_particles);
    gen_particles_parallel(seed, my_num_particles, my_particles_list, n, m);

    time_t start = time(NULL);

    printf("%d.Starting DLA\n", (int)my_rank);
    srand(42);

    for (int t = 1; t < ITERATIONS; t++)
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
                    move_parallel(p);
                    //p->path[t] = *p->current_position;
                    //p->size_path++;
                }
            }

        }
    }

    // FINALIZE //

    // free memory
    for (int i = 0; i < my_num_particles; i++)
    {
        if (my_particles_list[i].current_position != NULL)
            free(my_particles_list[i].current_position);
        //if (my_particles_list[i].path != NULL)
        //    free(my_particles_list[i].path);
    }
    if (my_particles_list != NULL)
        free(my_particles_list);
    time_t end = time(NULL);
    printf("%ld.Finished DLA -- time: %ld\n", my_rank, end - start);
    return NULL;
}




int main(int argc, char *argv[])
{

    get_args_pthreads(argv, &num_particles, &n, &m, seed, &num_threads);
    // printf("num_particles: %d, n: %d, m: %d, seed: %d, %d\n", num_particles, n, m, seed[0], seed[1]);
    // fflush(stdout);
    // num_particles = 50;
    // n = 10;
    // m = 10;
    // seed[0] = 50;
    // seed[1] = 50;

    printf("num_threads: %d \n", num_threads);

    matrix = (cell**)malloc(n*sizeof(cell*)); // Alloca un array di puntatori e inizializza tutti gli elementi a 0
    if (matrix == NULL)
        perror("Error allocating memory");

    for (int i = 0; i < n; i++)
    {
        matrix[i] = (cell*)malloc(m*sizeof(cell)); // Alloca un array di interi per ogni riga e inizializza tutti gli elementi a 0
        if (matrix[i] == NULL)
            perror("Error allocating memory");
        if (pthread_mutex_init(&matrix[i]->mutex, NULL))
            perror("Error initializing mutex");
        matrix[i]->value = 0;
    }

    matrix[seed[0]][seed[1]].value = 1; // set seed

    // create threads
    long thread;
    pthread_t* thread_handles;

    thread_handles = (pthread_t*) malloc (num_threads*sizeof(pthread_t)); 

    rand_seed = (unsigned int)time(NULL); 

    time_t start = time(NULL);
    // C'è il problema che ogni thread deve avere la sua matrice, quindi non posso passare la matrice come parametro
    // ma devo passare la matrice per ogni thread
    for (thread = 0; thread < num_threads; thread++)
        pthread_create(&thread_handles[thread], NULL, start_DLA_parallel, (void*)thread);

    for (thread = 0; thread < num_threads; thread++)
        pthread_join(thread_handles[thread], NULL);
   
    time_t end = time(NULL);

    printf("Elapsed time: %f seconds \n", (double)((end - start)));

    //write_matrix(n, m, matrix);
    //write_particles(num_particles, my_particles_list);

    // -----FINALIZE----- //


    printf("freed memory: ");
    for (int i = 0; i < n; i++)
    {
        if(matrix[i] != NULL){
            free(matrix[i]); // Libera la memoria della riga i-esima
        }
    }

    printf("matrix, ");
    if (matrix != NULL)
        free(matrix); // Libera la memoria dell'array di puntatori

    if (thread_handles != NULL)
        free(thread_handles);
    printf("thread_handles \n");


    return 0;
}