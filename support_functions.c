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

/*
 * Struttura per la posizione di una particella.
 */
typedef struct
{
    int x; // posizione sull'asse x
    int y; // posizione sull'asse y
} position;

/*
 * Struttura per la particella.
 * Contiene la posizione attuale, la direzione, lo stato di stuck e lo stato di uscita.
 * Lo stato di stuck indica se la particella è bloccata o meno.
 * Lo stato di uscita indica se la particella è uscita dalla matrice o meno.
 */
typedef struct
{
    position *current_position; // posizione attuale della particella

    int dire; // direzione

    int stuck; // 0 = not stuck, 1 = stuck
    int isOut; // 0 = inside, 1 = outside
} particle;

/*
 * Struttura per la cella della matrice.
 * Contiene il valore della cella e il lock per la mutua esclusione.
 * Il valore della cella indica se la cella è occupata e da cosa è occupata.
 * Vale 1 se è occupata da un seme, 2 o suo multiplo se è occupata da una particella.
 * Il lock è un intero per la mutua esclusione. Vale 0 se la cella è libera, 1 se è occupata.
 */
typedef struct
{
    int value; // 0 = libera, 1 = seed, 2 o più = particelle
} cell;

typedef struct
{
    particle *data;   // array di particelle
    int size;         // numero di particelle
    int capacity;     // capacità massima dell'array
    float coefficent; // coefficiente di reallocazione
} stuckedParticles;

void get_args_parallel(int argc, char *argv[], int *num_particles, int *n, int *m, int *seed, int *num_threads, int *horizon);
void get_args(int argc, char *argv[], int *num_particles, int *n, int *m, int *seed, int *horizon);
void write_matrix(int n, int m, int **matrix);
void print_matrix(int n, int m, int **matrix);
void move(particle *p, int n, int m);
void move_parallel(particle *p, int n, int m);
void move_pthread(particle *p, cell **matrix, int n, int m);
double get_time(void);
int init_StuckedParticles(stuckedParticles *sp, int capacity);
int sp_append(stuckedParticles *sp, particle p);
particle sp_pop(stuckedParticles *sp);
int sp_destroy(stuckedParticles *sp);

int init_StuckedParticles(stuckedParticles *sp, int capacity)
{
    sp->data = (particle *)malloc(capacity * sizeof(particle));
    if (sp->data == NULL)
    {
        return -1;
    }
    sp->size = 0;
    sp->capacity = (int)capacity;
    sp->coefficent = capacity;
    return 0;
}

int sp_append(stuckedParticles *sp, particle p)
{
    if (sp->size == sp->capacity - 1)
    {
        sp->data = (particle *)realloc(sp->data, (sp->capacity * 2) * sizeof(particle));
        if (sp->data == NULL)
        {
            return -1;
        }
        sp->capacity = (int)sp->capacity * 2;
    }
    sp->data[sp->size + 1] = p;
    sp->size++;

    return 0;
}

particle sp_pop(stuckedParticles *sp)
{
    if (sp->size == 0)
    {
        return (particle){NULL, 0, 0, 0};
    }
    particle p = sp->data[sp->size];
    sp->size--;
    return p;
}

int sp_destroy(stuckedParticles *sp)
{
    if (sp->data != NULL)
    {
        free(sp->data);
        return 0;
    }
    else
    {
        return -1;
    }
}

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