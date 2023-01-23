#include <stdio.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <math.h>

int main() {
    // Inizializzazione del generatore di numeri casuali
    const gsl_rng_type * T;
    gsl_rng * r;
    T = gsl_rng_default;
    r = gsl_rng_alloc (T);
  
    int x = 0; // Posizione iniziale
    double dt = 0.1; // Passo di simulazione
    double sigma = 1; // Deviazione standard

    for (int i = 0; i < 100; i++) {
        // Aggiungi una deviazione casuale normalmente distribuita alla posizione corrente
        x += (int )gsl_ran_gaussian(r, sigma) * sqrt(dt);
        printf("%f\n", x);
    }

    // Libera la memoria utilizzata dal generatore di numeri casuali
    gsl_rng_free (r);
    return 0;
}
