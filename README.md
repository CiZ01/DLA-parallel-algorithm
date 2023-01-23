# DLA Simulator
## Introduzione
DLA Simulator è un programma in C per simulare l'aggregazione di particelle che si muovono seguendo un moto Browniano in una superficie 2D. \
Presentiamo più implementazioni, una single core, una multi thread usando la libreria pthread e una multi thread usando OpenMP. \
Inoltre le nostre implementazioni prevedono il render delle fasi della simulazioni per mostrare come si aggregano le particelle.
## Come avviarlo
**Il progetto è valido solo se fatto girare su Linux, non si assicura il suo funzionamento su altre piattaforme visto l'utilizzo di librerie multi thread ed altro** \
Dopo aver clonato la repository è possibile avviare il programma a patto che le dipendenze vengano rispettate. (Vedi sezione <a href="Dipendenze"> Dipendenze</a>)
Quindi per compilarlo basta eseguire il seguente comando:

    gcc -o dla_single_thread.out dla_single_thread.c -lgd

Dove _-lgd_ è l'opzione necessaria per specificare la libreria GD che usiamo per il render delle immagini. \
\
Per le versioni multi thread va aggiunta l'opzione:

 - `-fopenmp` _per l'implementazione con OpenMP_
 - `-pthread` _per l'implementazione con pthread_

Per avviare la versione single thread basta eseguire il seguente comando e settare i giusti argomenti:

 - n,m _grandezza matrice, righe e colonne rispettivamente_
 - num_particles _numero di particelle da istanziare_
 - x,y _posizione del seed con le rispettive coordinate nella matrice_ [***opzionale***] \ Se non specificato sarà generato casualmente all'interno della superficie.

`./dla_single_thread.out n,m num_particles x,y`

Per le versioni multi thread:

 - OpenMP - `./dla_openmp.out n,m num_particles x,y num_threads`
 - pthread - `./dla_pthread.out n,m num_particles x,y num_threads` \
 In tutti e due i casi _num_threads_ sta per il numero di threads che si vuole [***opzionale***] \
 Se non specificato il valore di default è 4.

## Come funziona
Il programma è molto semplice, le tre implementazioni differiscono di poco, l'idea di fondo è sempre la stessa.
L' inizializzazione prevede il salvataggio dei dati passati come argomento al programma, l'allocazione della memoria per la matrice, che viene allocata e riempita interamente da 0, l'allocazione della memoria per la lista di particelle, queste vengono generate con posizione casuale. Dopo di che inizia la simulazione vera e propria. \
Tutta la simulazione è scandita da intervalli (l'unità di tempo la lasciamo scegliere all'utente), in ognuno di questi intervalli si scorre tutta la lista di particelle, e per ogni particella si eseguono due operazioni:

 1. Si controllano le celle adiacenti alla particella, nel caso sia presente una particella aggregata (o il seed iniziale) allora la particella in questione si aggregherà a sua volta e non verrà più considerata, in caso contrario si va alla seconda operazione.
 2. Si muove la particella di un fattore casuale in una direzione casuale cercando di simulare al meglio un moto Browniano. Quindi si ricomincia dal punto 1. 

Al termine dell'orizzonte di simulazione scelto viene salvato il risultato sottoforma di immagine in formato jpg, dopo di che viene liberata tutta la memoria allocata e nel caso delle implementazioni multi thread vengono eseguite le dovute operazioni di finalizzazione.

## Dipendenze
Per il render delle immagini in C abbiamo usato: [GD Graphics Library (libgd.github.io)](https://libgd.github.io/)
Per installarla su Ubuntu:
`sudo apt install libgd-dev`
