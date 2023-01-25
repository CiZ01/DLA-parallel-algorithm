#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <gd.h>
#include <errno.h>
#include <pthread.h>

unsigned gen_rand = 586;

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
    int isOut;      // 
} particle;

typedef struct
{
    int value;
    pthread_mutex_t mutex;
} cell;

void get_args_parallel(char *argv[], int *num_particles, int *n, int *m, int *seed, int *thread_count);
void get_args(char *argv[], int *num_particles, int *n, int *m, int *seed);
void write_matrix(int n, int m, int **matrix);
void write_paths(int num_particles, particle *particles_list);
void print_matrix(int n, int m, int **matrix);
void move(particle *part);
void move_parallel(particle *part);
void move_pthread(particle *p, cell** matrix, int n, int m);

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
 * Prende in input il numero di particelle e la lista di particelle.
 * Salva su un file di testo tutti i percorsi delle particelle.
 * Il file di testo sarà formattato come segue:
 *  - ogni riga rappresenta un particella
 *  - ogni colonna rappresenta un iterazione
 */
// void write_paths(int num_particles, particle *particles_list)
// {
//     FILE *fptr2;
//     fptr2 = fopen("output/paths.txt", "w+");
//     if (fptr2 == NULL)
//         perror("Error opening file");

//     for (int i = 0; i < num_particles; i++)
//     {
//         particle *p = &particles_list[i];
//         for (int j = 0; j < p->size_path; j++)
//         {
//             fprintf(fptr2, "%d,%d,", p->path[j].y, p->path[j].x);
//         }
//         fprintf(fptr2, "\n");
//     }

//     if (ferror(fptr2))
//         perror("Error writing file");

//     // close file
//     if (fclose(fptr2))
//         perror("Error closing file");
// }

/*
 * Recupera tutti gli argomenti passati in input al programma e li setta alle opportune variabili.
 * In caso di mancato argomento il programma termina per un segmentation fault.
 */
void get_args_parallel(char *argv[], int *num_particles, int *n, int *m, int *seed, int *thread_count)
{
    // get matrix dimensions
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

    // get number of threads
    *thread_count = (int)atoi(argv[4]);
}

void get_args(char *argv[], int *num_particles, int *n, int *m, int *seed)
{
    // get matrix dimensions
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
void move(particle *p)
{

    // move particle
    p->dire = rand() % 2 == 0 ? 1 : -1;
    p->current_position->x += rand() % 2 * p->dire;

    p->dire = rand() % 2 == 0 ? 1 : -1;
    p->current_position->y += rand() % 2 * p->dire;
}

void move_parallel(particle *p)
{

    // move particle
    p->dire = rand_r(&gen_rand) % 2 == 0 ? 1 : -1;
    p->current_position->x += rand_r(&gen_rand) % 2 * p->dire;

    p->dire = rand_r(&gen_rand) % 2 == 0 ? 1 : -1;
    p->current_position->y += rand_r(&gen_rand) % 2 * p->dire;
}

void move_pthread(particle *p, cell** matrix, int n, int m)
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

void createImage_intMatrix(gdImagePtr img, int width, int height, int** matrix) {
    printf("Creating image...\n");
    int black = gdImageColorAllocate(img, 0, 0, 0);
    int white = gdImageColorAllocate(img, 255, 255, 255);

    for (int y = 0; y<height; y++) {
        for (int x = 0; x < width; x++) {
            int color = matrix[y][x] == 0 ? white : black;
            gdImageSetPixel(img, x, y, color);
        }
    }
}

void createImage(gdImagePtr img, int width, int height, cell** matrix, char* filename) {
    int black = gdImageColorAllocate(img, 0, 0, 0);
    int red = gdImageColorAllocate(img, 255, 0, 0);

    for (int y = 0; y<height; y++) {
        for (int x = 0; x < width; x++) {
            if (matrix[y][x].value == 1) {
                gdImageSetPixel(img, x, y, black);
            } else if (matrix[y][x].value > 1){
                gdImageSetPixel(img, x, y, red);
            }
        }
    }
    FILE *out = fopen(filename, "wb");
    gdImageBmp(img, out, -1);
    fclose(out);
}

void saveImage(gdImagePtr img, char* filename){
    // Salva l'immagine
    FILE *out = fopen(filename, "wb");
    gdImageBmp(img, out, -1);
    fclose(out);
}
