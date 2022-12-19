#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int settings[] = {1,2};


typedef struct
{
    int x; // position on x axis
    int y; // position on y axis

    int vel;   // velocity
    int dire;  // direction

    int stuck; // 0 = not stuck, 1 = stuck
} particle;

//DA IMPLEMENTARE
typedef int (*start_dla)(int n, int m, int matrix[n][m], particle *p);
typedef void (*move)(particle p);
typedef int (*check_position)(int n, int m, int matrix[n][m], particle *p);


void gen_matrix(int n, int m, int matrix[n][m], int seed[2]);
void move(particle *part);
int gen_particles(int num_particles, particle particles_list[num_particles], char *particle_arg);
int start_DLA(int num_particles, particle particles_list[num_particles], int n, int m, int matrix[n][m]);
int check_position(int n, int m, int matrix[n][m], particle *p);

int check_position(int n, int m, int matrix[n][m], particle *p)
{
    if (p->stuck == 1)
    {
        return 0;
    }

    int directions[] = {0, 1, 0, -1, 1, 0, -1, 0, 1, 1, 1, -1, -1, 1, -1, -1};
    for (int i = 0; i < 8; i+=2)
    {
        int near_x = p->x + directions[i];
        int near_y = p->y + directions[i + 1];

        if (near_x > 0 || near_x < n || near_y > 0 || near_y < m)
        {
            if (matrix[near_x][near_y] == 1)
            {
                matrix[p->x][p->y] = 1;
                p->stuck = 1;
                return -1;
            }
        }
    }
    return 0;
}

void move(particle *p)
{
    // move particle
    p->x += p->vel*((rand()%2)-1);
    p->y += p->vel*((rand()%2)-1);
}

int gen_particles(int num_particles, particle particles_list[num_particles], char *particle_arg)
{
    // get data from particle_arg
    char *token = strtok(particle_arg, ",");
    int index = 0;
    while (token != NULL)
    {
        if (index == num_particles)
        {
            printf("Error: too many particles\n");
            return 1;
        }

        // get particle data
        int i = (int)atoi(token);
        token = strtok(NULL, ",");
        int j = (int)atoi(token);
        token = strtok(NULL, ",");
        int v = (int)atoi(token);
        token = strtok(NULL, ",");

        // create particle
        particle *p = malloc(sizeof(particle));
        p->x = i;
        p->y = j;
        p->vel = v;

        // add particle to list
        particles_list[index] = *p;
        index++;
    }
    return 0;
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

int start_DLA(int num_particles,
              particle particles_list[num_particles],
              int n, int m,
              int matrix[n][m])
{
    printf("Starting DLA\n");
    print_matrix(n, m, matrix);
    int i;
    for (i = 0; i < num_particles; i++)
    {
        particle *p = &particles_list[i];
        printf("Particle %d: (%d, %d) \n", i, p->x, p->y);
        if (p->stuck == 0)
        {
            
            if(check_position(n, m, matrix, p) == 0){move(p);}
        }
    }
    printf("Finished DLA\n");
    return 0;
}

int main(int argc, char *argv[])
{
    int n, m;    // matrix dimensions
    int seed[2]; // seed position

    // get matrix dimensions
    char *sizes = argv[1];
    char *token = strtok(sizes, ",");
    n = (int)atoi(token);
    token = strtok(NULL, ",");
    m = (int)atoi(token);

    // get seed position
    char *seed_pos = argv[2];
    token = strtok(seed_pos, ",");
    seed[0] = (int)atoi(token);
    token = strtok(NULL, ",");
    seed[1] = (int)atoi(token);

    // create matrix
    int matrix[n][m];
    gen_matrix(n, m, matrix, seed);

    // get particle data
    char *particle_arg = argv[3];
    int num_particles = (int)atoi(argv[4]);
    particle particles_list[num_particles];
    // create particles and check for errors
    if (gen_particles(num_particles, particles_list, particle_arg))
    {
        printf("Error: could not create particles\n");
        exit(1);
    }

    // start DLA
    start_DLA(num_particles, particles_list, n, m, matrix);
    print_matrix(n, m, matrix);

    return 0;
}