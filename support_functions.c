#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h> //solo perché vscode da errore su optarg
#include <string.h>
#include <time.h>
#include <gd.h>
#include <errno.h>
#include <pthread.h>

#define NUM_THREADS 4
#define HORIZON 1000

unsigned int gen_rand = 586761;
int seed_rand = 586761;

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

    int stuck; // 0 = not stuck, 1 = stuck
    int isOut; //
} particle;

typedef struct
{
    int value;
    pthread_mutex_t mutex;
} cell;

void get_args_parallel(int argc, char *argv[], int *num_particles, int *n, int *m, int *seed, int *num_threads, int *horizon);
void get_args(int argc, char *argv[], int *num_particles, int *n, int *m, int *seed, int *horizon);
void write_matrix(int n, int m, int **matrix);
void write_paths(int num_particles, particle *particles_list);
void print_matrix(int n, int m, int **matrix);
void move(particle *p, int n, int m);
void move_parallel(particle *part, int n, int m);
void move_pthread(particle *p, cell **matrix, int n, int m);

void write_matrix(int n, int m, int **matrix)
{
    FILE *fptr;

    fptr = fopen("output/matrix.txt", "w+");
    if (fptr == NULL)
        perror("Error opening file");

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            fprintf(fptr, "%d ", matrix[i][j]);
        }
        fprintf(fptr, "\n");
    }

    if (ferror(fptr))
        perror("Error writing file");

    // close file
    if (fclose(fptr))
        perror("Error closing file");
}

/*
 * Recupera tutti gli argomenti passati in input al programma e li setta alle opportune variabili.
 * In caso di mancato argomento il programma termina per un segmentation fault.
 */
void get_args_parallel(int argc, char *argv[], int *num_particles, int *n, int *m, int *seed, int *num_threads, int *horizon)
{
    // get matrix dimensions
    char *token = strtok(argv[1], ",");
    *n = (int)atoi(token);
    token = strtok(NULL, ",");
    *m = (int)atoi(token);

    // get number of particles
    *num_particles = (int)atoi(argv[2]);

    // get seed position
    seed[0] = (int)rand_r(&gen_rand) % *n;
    seed[1] = (int)rand_r(&gen_rand) % *m;

    *horizon = HORIZON;
    *num_threads = NUM_THREADS;

    int opt;
    while ((opt = getopt(argc, argv, "-n:-t:-s:")) != -1)
    {
        switch (opt)
        {
        case 'n':
            *num_threads = atoi(optarg);
            break;
        case 't':
            *horizon = atoi(optarg);
            break;
        case 's':
            // get seed position
            token = strtok(optarg, ",");
            seed[0] = (int)atoi(token);
            token = strtok(NULL, ",");
            seed[1] = (int)atoi(token);
            break;
        case ':':
            printf("Opzione richiede un argomento: %c", opt);
            break;
        case '?':
            printf("Opzione non valida: %c\n", opt);
            break;
        }
    }
}

void get_args(int argc, char *argv[], int *num_particles, int *n, int *m, int *seed, int *horizon)
{
    // get matrix dimensions
    char *token = strtok(argv[1], ",");
    *n = (int)atoi(token);
    token = strtok(NULL, ",");
    *m = (int)atoi(token);

    // get number of particles
    *num_particles = (int)atoi(argv[2]);

    // get seed position
    seed[0] = (int)rand_r(&gen_rand) % *n;
    seed[1] = (int)rand_r(&gen_rand) % *m;

    *horizon = HORIZON;

    int opt;
    while ((opt = getopt(argc, argv, "-t:-s:")) != -1)
    {
        switch (opt)
        {
        case 't':
            *horizon = atoi(optarg);
            break;
        case 's':
            // get seed position
            token = strtok(optarg, ",");
            seed[0] = (int)atoi(token);
            token = strtok(NULL, ",");
            seed[1] = (int)atoi(token);
            break;
        case ':':
            printf("Opzione richiede un argomento: %c", opt);
            break;
        case '?':
            printf("Opzione non valida: %c\n", opt);
            break;
        }
    }
}

/*
 * print_matrix stampa la matrice.
 * La funzione riceve in input le dimensioni della matrice e la matrice.
 * La funzione non ritorna nulla.
 */
void print_matrix(int n, int m, int **matrix)
{
    int a, b;
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
 * move muove la particella in una direzione pseudocasuale.
 * La funzione riceve in input la particella interessata.
 * La funzione non ritorna nulla.
 */
void move(particle *p, int n, int m)
{

    // move particle
    p->dire = rand() % 2 == 0 ? 1 : -1;
    p->current_position->x += rand() % 2 * p->dire;

    p->dire = rand() % 2 == 0 ? 1 : -1;
    p->current_position->y += rand() % 2 * p->dire;

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

void move_parallel(particle *p, int n, int m)
{

    // move particle
    p->dire = rand_r(&gen_rand) % 2 == 0 ? 1 : -1;
    p->current_position->x += rand_r(&gen_rand) % 2 * p->dire;

    p->dire = rand_r(&gen_rand) % 2 == 0 ? 1 : -1;
    p->current_position->y += rand_r(&gen_rand) % 2 * p->dire;

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

void move_pthread(particle *p, cell **matrix, int n, int m)
{

    // move particle
    p->dire = rand_r(&gen_rand) % 2 == 0 ? 1 : -1;
    p->current_position->x += rand_r(&gen_rand) % 2 * p->dire;

    p->dire = rand_r(&gen_rand) % 2 == 0 ? 1 : -1;
    p->current_position->y += rand_r(&gen_rand) % 2 * p->dire;

    if (!(p->current_position->x >= 0 && p->current_position->x < m && p->current_position->y >= 0 && p->current_position->y < n))
    {
        p->isOut = 1;
        return;
    }
    else
    {
        p->isOut = 0;
        matrix[p->current_position->y][p->current_position->x].value += 2;
    }
}

void write_matrix_cell(int n, int m, cell **matrix)
{
    FILE *fptr;

    fptr = fopen("output/matrix.txt", "a");
    if (fptr == NULL)
        perror("Error opening file");

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            fprintf(fptr, "%d ", matrix[i][j].value);
        }
        fprintf(fptr, "\n");
    }
    fprintf(fptr, "\n");

    if (ferror(fptr))
        perror("Error writing file");

    // close file
    if (fclose(fptr))
        perror("Error closing file");
}

void createImage_intMatrix(gdImagePtr img, int width, int height, int **matrix, int *colors, char *filename)
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (matrix[y][x] == 1)
            {
                gdImageSetPixel(img, x, y, colors[0]); // set black
            }
            else if (matrix[y][x] > 1)
            {
                gdImageSetPixel(img, x, y, colors[1]); // set red
            }
        }
    }

    // Salva l'immagine
    FILE *out = fopen(filename, "wb");
    gdImageJpeg(img, out, 100);
    fclose(out);
}

void createImage(gdImagePtr img, int width, int height, cell **matrix, char *filename, int *colors)
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (matrix[y][x].value == 1)
            {
                gdImageSetPixel(img, x, y, colors[0]);
            }
            else if (matrix[y][x].value > 1)
            {
                gdImageSetPixel(img, x, y, colors[1]);
            }
        }
    }
    FILE *out = fopen(filename, "wb");
    gdImageJpeg(img, out, 100);
    fclose(out);
}