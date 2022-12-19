#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 
 * Questa struct rappresenta la configurazione con cui verrà inizializzato l'algoritmo DLA.
 * Presenta tutte le caratteristiche che possono essere modificate e che andranno a 
 * descrivere il comportamento dell'algoritmo.
 */
typedef struct{
    int movement;
    int spawn;
}config;

int read_config(char* filename, config *conf){
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    int read;
    fp = fopen(filename, "r");
    if (fp == NULL)
        return 1;
    while ((read = getline(&line, &len, fp)) != -1) {
        if(strstr(line, "movement") != NULL){
            char *token = strtok(line, ":");
            token = strtok(NULL, ":");
            conf->movement = atoi(token);
        }
        if(strstr(line, "spawn") != NULL){
            char *token = strtok(line, " ");
            token = strtok(NULL, " ");
            conf->spawn = atoi(token);
        }
    }
    fclose(fp);
    if (line)
        free(line);
    return 0;
}

int main(){
    config conf;
    int err = read_config("config.txt", &conf);
    if (err){
        printf("Error while reading configuration file");
        return 1;
    }
    printf("Movement: %d, spawn: %d \n", conf.movement, conf.spawn);
    return 0;
}