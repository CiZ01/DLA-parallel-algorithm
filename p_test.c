#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct{
    int value;
    pthread_mutex_t mutex;
} cell;

typedef struct{
    int x;
    int y;
} particle;


int main(int argc, char *argv[]){
    int n = 10;
    int m = 10;

    cell **matrix = (cell**)malloc(n*sizeof(cell*)); // Alloca un array di puntatori e inizializza tutti gli elementi a 0
    if (matrix == NULL)
        perror("Error allocating memory");
    for(int i = 0; i < n; i++){
        matrix[i] = (cell*)malloc(m*sizeof(cell)); // Alloca un array di interi per ogni riga e inizializza tutti gli elementi a 0
        if (matrix[i] == NULL)
            perror("Error allocating memory");
        if (pthread_mutex_init(&matrix[i]->mutex, NULL))
            perror("Error initializing mutex");
        matrix[i]->value = 0;
    }
    particle p = {3, 2};

    particle* c = &p;

    matrix[c->x][c->y].value = 1; // set seed


    for(int i = 0; i < n; i++){
        for(int j = 0; j < m; j++){
            printf("%d ", matrix[i][j].value);
        }
        printf("\n");
    }
    for(int i = 0; i < n; i++){
        pthread_mutex_destroy(&matrix[i]->mutex);
        free(matrix[i]);
    }
    free(matrix);
    return 0;
}