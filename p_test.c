#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>



void *hello(void* rank);
typedef struct{
    int x;
}pippo;

typedef struct{
    pippo *a;
} position;


position* list;


int main(){

    pthread_t *threads;
    int num_threads = 4;
    threads = (pthread_t*)malloc(num_threads*sizeof(pthread_t));
    list = (position*)malloc(10*sizeof(position));

    for (int i=0; i<10; i++){
        position *p;
        p = (position*)malloc(sizeof(position));
        p->a = (pippo*)malloc(sizeof(pippo));
        p->a->x = i;

        list[i] = *p;
    }


    for(long i =0; i<num_threads; i++){
        pthread_create(&threads[i], NULL, hello, (void*)i);
    }

    for(int i=0; i<num_threads; i++){
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(list);


    return 0;

}

void *hello(void* rank){
    long my_rank = (long)rank;

    position a = list[my_rank];

    printf("Hello from thread %ld, x: %d \n", my_rank, a.a->x);

    return NULL;
}