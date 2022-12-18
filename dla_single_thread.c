#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void gen_matrix(int n, int m, int matrix[n][m], int seed[2]);

typedef struct
{
    int x; // position on x axis
    int y; // position on y axis

    int vel;   // velocity
    int stuck; // 0 = not stuck, 1 = stuck
} particle;

void move(particle *part);

void move(particle *part)
{
    // move particle
    part->x += part->vel;
    part->y += part->vel;
}

void gen_matrix(int n, int m, int matrix[n][m], int seed[2])
{
    int i, j;
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < m; j++)
        {
            matrix[i][j] = 0;
        }
    }
    matrix[seed[0]][seed[1]] = 1;
}

void print_matrix(int n, int m, int matrix[n][m])
{
    int i, j;
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < m; j++)
        {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    int a;
    int matrix[10][10];
    int seed[2] = {5, 5};
    gen_matrix(10, 10, matrix, seed);
    print_matrix(10, 10, matrix);
    particle part = {5, 5, 1, 0};
    move(&part);
    printf("%d, %d \n", part.x, part.y);
    return 0;
}