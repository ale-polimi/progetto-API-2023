#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define DEBUG 1

#define FALSE 0
#define TRUE (!FALSE)
#define FWD 0
#define REV (!FWD)
#define NUM_OF_CARS 512
#define BUFF_LEN 7710 //Max caratteri ammissibili per 512 numeri a 32 bit + spazi + comando "aggiungi-stazione"

/*
 * Comandi:
 * - aggiungi-stazione distanza numero-auto autonomia-auto-1 ... autonomia-auto-n
 * - demolisci-stazione distanza
 * - aggiungi-auto distanza-stazione autonomia-auto-da-aggiungere
 * - rottama-auto distanza-stazione autonomia-auto-da-rottamare
 * - pianifica-percorso distanza-stazione-partenza distanza-stazione-arrivo
 */

/**
 * Rappresentazione della stazione di servizio. Contiene la distanza dall'inizio dell'autostrada e il parco veicoli disponibile nella stazione.
 */
typedef struct station {
    uint32_t distance;
    uint32_t vehiclesInStation[NUM_OF_CARS];
    struct station *previous;
    struct station *next;
} t_station;

typedef t_station* ptr_station;
typedef int bool;
typedef int direction;


// Struttura per rappresentare un nodo
struct Node {
    int vertex;     // Indice del nodo
    int distance;   // Distanza minima dal nodo di origine
    int steps;      // Numero di passi dal nodo di origine
} t_node;
void dijkstra(int **graph, int src, int dest, int *lut, int numOfStations);
void swapHeap(struct Node* x, struct Node* y);
void printSolution(int dist[], int parent[], int src, int *lut, int);
int minDistance(uint32_t dist[], bool visited[], int numOfStations);







ptr_station addStation(ptr_station, uint32_t, uint32_t*);
ptr_station removeStation(ptr_station, uint32_t);
ptr_station destroyStations(ptr_station);
void addVehicle(ptr_station, uint32_t, uint32_t);
void removeVehicle(ptr_station, uint32_t, uint32_t);

int findNumOfStations(ptr_station, uint32_t, uint32_t, direction);

void printStations(ptr_station);

void safe_fgets(char*);

// Sorting functions.
void sortVehicles(uint32_t*, int, int);
int partition(uint32_t*, int, int);
void swap(uint32_t*, uint32_t*);

ptr_station findStation(ptr_station, uint32_t);
void resetArray(bool *, int);

void createGraph(ptr_station, ptr_station, int, uint32_t *, uint32_t**);
int indexOf(uint32_t *lut, int length, uint32_t distance);

void printMatrix(uint32_t **pInt, int stations);

int main() {
    ptr_station autostrada;
    char *inputBuffer = (char *) malloc(BUFF_LEN * sizeof(char));
    autostrada = NULL;
    direction direction;

    while(TRUE){
        safe_fgets(inputBuffer);
        if(inputBuffer[0] == 'a'){
            if(inputBuffer[9] == 's'){
                /* Comando: aggiungi-stazione */

                int i = 0;
                uint32_t tempDistance;
                uint32_t tempVehicles[512] = { 0 };
                char* p = inputBuffer + 18;
                while(p < inputBuffer+BUFF_LEN ) {
                    char *end;
                    uint32_t tempValue = strtol(p, &end, 10);
                    if (tempValue == 0L && end == p)  //docs also suggest checking errno value
                        break;

                    if(i == 0){
                        tempDistance = tempValue;
                    } else {
                        tempVehicles[i - 1] = tempValue;
                    }
                    i++;

                    p = end;
                }

                autostrada = addStation(autostrada, tempDistance, tempVehicles);

            } else if(inputBuffer[9] == 'a'){
                /* Comando: aggiungi-auto */

                int i = 0;
                uint32_t tempDistance;
                uint32_t tempVehicle;
                char* p = inputBuffer + 14;
                while(p < inputBuffer+BUFF_LEN ) {
                    char *end;
                    uint32_t tempValue = strtol(p, &end, 10);
                    if (tempValue == 0L && end == p)  //docs also suggest checking errno value
                        break;

                    if(i == 0){
                        tempDistance = tempValue;
                    } else {
                        tempVehicle = tempValue;
                    }
                    i++;

                    p = end;
                }

                addVehicle(autostrada, tempDistance, tempVehicle);
            }
        } else if(inputBuffer[0] == 'd'){
            /* Comando: demolisci-stazione */

            uint32_t tempDistance;
            char* p = inputBuffer + 19;
            while(p < inputBuffer+BUFF_LEN ) {
                char *end;
                uint32_t tempValue = strtol(p, &end, 10);
                if (tempValue == 0L && end == p)  //docs also suggest checking errno value
                    break;

                tempDistance = tempValue;

                p = end;
            }

            autostrada = removeStation(autostrada, tempDistance);

        } else if(inputBuffer[0] == 'r'){
            /* Comando: rottama-auto */

            int i = 0;
            uint32_t tempDistance;
            uint32_t tempVehicle;
            char* p = inputBuffer + 13;
            while(p < inputBuffer+BUFF_LEN ) {
                char *end;
                uint32_t tempValue = strtol(p, &end, 10);
                if (tempValue == 0L && end == p)  //docs also suggest checking errno value
                    break;

                if(i == 0){
                    tempDistance = tempValue;
                } else {
                    tempVehicle = tempValue;
                }
                i++;

                p = end;
            }

            removeVehicle(autostrada, tempDistance, tempVehicle);

        } else if(inputBuffer[0] == 'p'){
            /* Comando: pianifica-percorso */

            int i = 0;
            uint32_t from;
            uint32_t to;
            char* p = inputBuffer + 19;
            while(p < inputBuffer+BUFF_LEN ) {
                char *end;
                uint32_t tempValue = strtol(p, &end, 10);
                if (tempValue == 0L && end == p)  //docs also suggest checking errno value
                    break;

                if(i == 0){
                    from = tempValue;
                } else {
                    to = tempValue;
                }
                i++;

                p = end;
            }

            if(from < to){
                /* Left to right */
                direction = FWD;
            } else {
                /* Right to left */
                direction = REV;
            }

            if(from == to){
                printf("%u\n", from);
            } else {
                int numOfStations = 0;
                numOfStations = findNumOfStations(autostrada, from, to, direction);

                switch (direction) {
                    case FWD:
                        if(numOfStations == 0){
                            ptr_station ptTemp;

                            ptTemp = findStation(autostrada, from);

                            if(ptTemp->next->distance - ptTemp->distance <= ptTemp->vehiclesInStation[0]){
                                printf("%u %u\n", ptTemp->distance, ptTemp->next->distance);
                            } else {
                                printf("nessun percorso\n");
                            }
                        } else {

#ifdef DEBUG
printf("NumOfStations: %d\n", numOfStations + 2);
#endif

                            ptr_station startStation = findStation(autostrada, from);
                            ptr_station endStation = findStation(autostrada, to);
                            uint32_t* lut = malloc((numOfStations + 2)*sizeof(uint32_t));
                            int k;

                            /* Creo una LUT per trasformare le distanze delle stazioni in numeri da 0 a numOfStations */
                            for(k = 0; k < numOfStations + 2; k++){
                                lut[k] = 0;
                            }

                            ptr_station ptrTemp = startStation;
                            k = 0;
                            while(ptrTemp != endStation){
                                lut[k] = ptrTemp->distance;
                                k++;
                                ptrTemp = ptrTemp->next;
                            }
                            lut[k] = ptrTemp->distance;


                            /* Creare la matrice di adiacenza per il grafo */
                            uint32_t** adjacencyMatrix = (uint32_t**)malloc((numOfStations + 2)*sizeof(uint32_t*));
                            for(k = 0; k < numOfStations + 2; k++){
                                adjacencyMatrix[k] = (uint32_t*)malloc((numOfStations + 2)*sizeof(uint32_t));
                                for(int j = 0; j < numOfStations + 2; j++){
                                    adjacencyMatrix[k][j] = 0;
                                }
                            }

                            /* Popolare la matrice di adiacenza */
                            createGraph(startStation, endStation, numOfStations + 2, lut, adjacencyMatrix);
/*
#ifdef DEBUG
printMatrix(adjacencyMatrix, numOfStations + 2);
#endif
 */

                            dijkstra(adjacencyMatrix, indexOf(lut, numOfStations + 2, startStation->distance), indexOf(lut, numOfStations + 2, endStation->distance), lut, numOfStations + 2);

                            for(k = 0; k < numOfStations + 2; k++){
                                free(adjacencyMatrix[k]);
                            }
                            free(adjacencyMatrix);
                            free(lut);
                        }
                        break;
                    case REV:
                        if(numOfStations == 0){
                            ptr_station ptTemp;

                            ptTemp = findStation(autostrada, from);

                            if(ptTemp->distance - ptTemp->previous->distance <= ptTemp->vehiclesInStation[0]){
                                printf("%u %u\n", ptTemp->distance, ptTemp->previous->distance);
                            } else {
                                printf("nessun percorso\n");
                            }
                        } else {
                            /* Funzione ricorsiva */



                        }
                        break;
                }


            }

        } else {
            if(autostrada != NULL){
                autostrada = destroyStations(autostrada);
            }
            if(inputBuffer != NULL){
                free(inputBuffer);
            }
            break;
        }
    }

    return 0;
}

void printMatrix(uint32_t **graph, int length) {
    for(int row = 0; row < length; row++){
        for(int col = 0; col < length; col++){
            if(col == length - 1){
                printf("| %d |\n", graph[row][col]);
            } else {
                printf("| %d ", graph[row][col]);
            }
        }
    }

}
/*
 * =========================================
 */


int minDistance(uint32_t dist[], bool visited[], int numOfStations) {
    int min = INT_MAX, min_index;

    for (int v = 0; v < numOfStations; v++) {
        if (!visited[v] && dist[v] <= min) {
            min = dist[v];
            min_index = v;
        }
    }

    return min_index;
}

// Funzione di utilità per scambiare due nodi
void swapHeap(struct Node* x, struct Node* y) {
    struct Node temp = *x;
    *x = *y;
    *y = temp;
}

// Funzione per eseguire heapify in base al numero di passi
void minHeapify(struct Node heap[], int size, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < size && heap[left].steps < heap[smallest].steps)
        smallest = left;

    if (right < size && heap[right].steps < heap[smallest].steps)
        smallest = right;

    if (smallest != idx) {
        swapHeap(&heap[idx], &heap[smallest]);
        minHeapify(heap, size, smallest);
    }
}

// Funzione per mantenere la proprietà di min-heap
void buildMinHeap(struct Node heap[], int size) {
    int i;
    for (i = (size - 1) / 2; i >= 0; i--)
        minHeapify(heap, size, i);
}

// Funzione per estrarre il nodo minimo dalla coda di priorità
struct Node extractMin(struct Node heap[], int* size) {
    struct Node minNode = heap[0];
    heap[0] = heap[(*size) - 1];
    (*size)--;
    minHeapify(heap, *size, 0);
    return minNode;
}

// Funzione di utilità per aggiornare la distanza e il numero di passi di un nodo
void updateDistanceAndSteps(struct Node heap[], int vertex, int newDistance, int newSteps, int* size) {
    int i;
    for (i = 0; i < *size; i++) {
        if (heap[i].vertex == vertex) {
            heap[i].distance = newDistance;
            heap[i].steps = newSteps;
            break;
        }
    }
    buildMinHeap(heap, *size);
}

// Funzione di utilità per trovare il nodo con la distanza minima e il numero di passi più basso
int minDistanceWithSteps(struct Node heap[], int size) {
    int minDist = INT_MAX;
    int minSteps = INT_MAX;
    int minVertex = -1;

    for (int i = 0; i < size; i++) {
        if (heap[i].distance < minDist) {
            minDist = heap[i].distance;
            minSteps = heap[i].steps;
            minVertex = heap[i].vertex;
        } else if (heap[i].distance == minDist && heap[i].steps < minSteps) {
            minSteps = heap[i].steps;
            minVertex = heap[i].vertex;
        }
    }

    return minVertex;
}

// Funzione di utilità per trovare l'indice di un nodo nella coda di priorità
int findNodeIndex(struct Node heap[], int vertex, int size) {
    for (int i = 0; i < size; i++) {
        if (heap[i].vertex == vertex) {
            return i;
        }
    }
    return -1;
}

// Funzione per stampare il percorso dal nodo di origine al nodo di destinazione
void printPath(int parent[], int j, int *lut) {
    if (parent[j] == -1)
        return;

    printPath(parent, parent[j], lut);
    printf("%d ", lut[j]);
}

void dijkstra(int **graph, int src, int dest, int *lut, int numOfStations) {
    uint32_t *dist = malloc(numOfStations*sizeof(uint32_t));      // Distanza minima dal nodo di origine
    bool *visited = malloc(numOfStations*sizeof(bool));
    // int *steps = malloc(numOfStations*sizeof(int));     // Numero di passi dal nodo di origine
    int *parent = malloc(numOfStations*sizeof(int));    // Percorso ottimo

    // Coda di priorità per gestire i nodi in base alla distanza e al numero di passi
    // struct Node *priorityQueue = malloc(numOfStations*sizeof(t_node));
    // int pqSize = numOfStations;

    // Inizializza le strutture dati
    for (int i = 0; i < numOfStations; i++) {
        dist[i] = UINT32_MAX;
        visited[i] = FALSE;
        // steps[i] = INT_MAX;
        parent[i] = -1;
        /*
        priorityQueue[i].vertex = i;
        priorityQueue[i].distance = INT_MAX;
        priorityQueue[i].steps = INT_MAX;
         */
    }

    // La distanza dal nodo di origine a se stesso è sempre 0
    dist[src] = 0;
    // steps[src] = 0;
    // updateDistanceAndSteps(priorityQueue, src, 0, 0, &pqSize);

    for(int count = 0; count < numOfStations - 1; count++){
        int u = minDistance(dist, visited, numOfStations);
        visited[u] = TRUE;

        for(int v = 0; v < numOfStations; v++){
            if(u >= 0){
                if(!visited[v] && graph[u][v] && dist[u] + graph[u][v] < dist[v]){
                    parent[v] = u;
                    dist[v] = dist[u] + graph[u][v];
                }
            }
        }
    }

    /*
    // Trova il percorso ottimo
    while (pqSize > 0) {
        int u = minDistanceWithSteps(priorityQueue, pqSize);
        int uIndex = findNodeIndex(priorityQueue, u, pqSize);

        // Estrai il nodo minimo dalla coda di priorità
        struct Node uNode = extractMin(priorityQueue, &pqSize);

        for (int v = 0; v < numOfStations; v++) {
            if(u >= 0){
                if (graph[u][v] && dist[u] + graph[u][v] < dist[v]) {
                    dist[v] = dist[u] + graph[u][v];
                    steps[v] = steps[u] + 1;
                    parent[v] = u;
                    updateDistanceAndSteps(priorityQueue, v, dist[v], steps[v], &pqSize);
                }
            }
        }
    }
     */

    // Stampa la soluzione
    //printSolution(dist, parent, src, lut, numOfStations);
    int foundPath = 1;
    for(int k = 0; k < numOfStations; k++){
        if(dist[k] == UINT32_MAX){
            printf("nessun percorso");
            foundPath = 0;
            break;
        }
    }
    if(foundPath){
        printf("%u ", lut[src]);
        printPath(parent, dest, lut);
    }
    printf("\n");

    free(dist);
    free(visited);
    // free(steps);
    free(parent);
    // free(priorityQueue);
}

/*
 * =========================================
 */

void printSolution(int dist[], int parent[], int src, int *lut, int numOfStations) {
    printf("Nodo\tDistanza\tPercorso\n");
    for (int i = 0; i < numOfStations; i++) {
        printf("%d\t%d\t%d ", lut[i], dist[i], lut[src]);
        printPath(parent, i, lut);
        printf("\n");
    }
}

int indexOf(uint32_t *lut, int length, uint32_t distance) {
    for(int i = 0; i < length; i++){
        if(lut[i] == distance){
            return i;
        }
    }

    return -1;
}

void createGraph(ptr_station departure, ptr_station destination, int numOfStations, uint32_t * lut, uint32_t **adjacencyMatrix){

    if(departure == destination){
        return;
    }

    ptr_station ptrTemp = departure;
    int k = 0;
    while(ptrTemp != destination){
        if(ptrTemp != departure){
            if(ptrTemp->distance - departure->distance <= departure->vehiclesInStation[0]){
                adjacencyMatrix[indexOf(lut, numOfStations, departure->distance)][indexOf(lut, numOfStations, ptrTemp->distance)] = ptrTemp->distance - departure->distance;
            }
        }
        if(ptrTemp->next == destination){
            if(ptrTemp->next->distance - departure->distance <= departure->vehiclesInStation[0]){
                adjacencyMatrix[indexOf(lut, numOfStations, departure->distance)][indexOf(lut, numOfStations, ptrTemp->next->distance)] = ptrTemp->distance - departure->distance;
            }
        }
        ptrTemp = ptrTemp->next;
    }
    createGraph(departure->next, destination, numOfStations, lut, adjacencyMatrix);
}

/**
 * Funzione per ricevere in modo sicuro l'input.
 * @param s è il buffer per lo stream di input.
 */
void safe_fgets(char *s){
    if(fgets(s, BUFF_LEN, stdin) == NULL){
        exit(EXIT_FAILURE);
    }
}

/**
 * Metodo utilizzato per stampare in sequenza le stazioni e i contenuti. Usato per debug.
 * @param ptStations è il puntatore all'inizio dell'autostrada.
 */
void printStations(ptr_station ptStations){
    ptr_station ptTemp;

    ptTemp = ptStations;
    while(ptTemp != NULL){
        printf("Stazione al km: %u\n", ptTemp->distance);
        for(int i = 0; i < NUM_OF_CARS; i++){
            printf("\t%u\n", ptTemp->vehiclesInStation[i]);
        }
        ptTemp = ptTemp->next;
    }
}

/**
 * Aggiunta in ordine crescente di una stazione all'autostrada. Se è già presente una stazione di servizio alla distanza specificata, non la si aggiunge.
 * @param ptStations è il puntatore all'inizio della lista.
 * @param distance è la distanza della nuova stazione dall'inizio dell'autostrada.
 * @param vehicles è il puntatore all'array contenente i veicoli della nuova stazione.
 * @return il puntatore all'inizio della lista aggiornata.
 */
ptr_station addStation(ptr_station ptStations, uint32_t distance, uint32_t* vehicles){
    ptr_station ptTempToAdd;
    ptr_station ptTemp;
    ptr_station ptTempPrevious;
    bool alreadyPresent = FALSE;
    int i;

    ptTempToAdd = malloc(sizeof (t_station));
    if(!ptTempToAdd){
        printf("Errore allocazione memoria.\n");
    } else {
        ptTempToAdd->distance = distance;
        for(i = 0; i < NUM_OF_CARS; i++){
            ptTempToAdd->vehiclesInStation[i] = vehicles[i];
        }
        sortVehicles(ptTempToAdd->vehiclesInStation, 0, NUM_OF_CARS - 1);

        ptTempToAdd->previous = NULL;
        ptTempToAdd->next = NULL;

        if(ptStations == NULL){
            ptStations = ptTempToAdd;
        } else {
            /* Aggiunta del nodo in ordine crescente */
            ptTemp = ptStations;
            ptTempPrevious = NULL;
            while(ptTemp != NULL && ptTemp->distance < ptTempToAdd->distance){
                ptTempPrevious = ptTemp;
                ptTemp = ptTemp->next;
            }

            if(ptTemp != NULL && ptTemp->distance == ptTempToAdd->distance){
                alreadyPresent = TRUE;
            }

            if(!alreadyPresent) {
                if (ptTempPrevious == NULL) {
                    /* Aggiungo in testa */
                    ptStations->previous = ptTempToAdd;
                    ptTempToAdd->next = ptStations;
                    ptStations = ptTempToAdd;
                } else if (ptTemp == NULL) {
                    /* Aggiungo in coda */
                    /* Collego il nodo precedente al nuovo nodo */
                    ptTempPrevious->next = ptTempToAdd;
                    /* Collego il nuovo nodo al nodo precedente */
                    ptTempToAdd->previous = ptTempPrevious;
                    /* Collego il nuovo nodo al successivo */
                    ptTempToAdd->next = ptTemp;
                } else {
                    /* Collego il nodo precedente al nuovo nodo */
                    ptTempPrevious->next = ptTempToAdd;
                    /* Collego il nuovo nodo al nodo precedente */
                    ptTempToAdd->previous = ptTempPrevious;
                    /* Collego il nuovo nodo al successivo */
                    ptTempToAdd->next = ptTemp;
                    /* Collego il nodo successivo al nuovo nodo */
                    ptTemp->previous = ptTempToAdd;
                }
            }
        }

        if(!alreadyPresent){
            printf("aggiunta stazione\n");
        } else {
            printf("non aggiunta stazione\n");
            free(ptTempToAdd);
        }
    }

    return(ptStations);
}

/**
 * Rimozione di una stazione di servizio. Se non è presente una stazione alla distanza specificata, non la si demolisce.
 * @param ptStations è il puntatore all'inizio dell'autostrada.
 * @param distance è la distanza alla quale è presente la stazione da demolire.
 * @return il puntatore all'inizio della lista aggiornata.
 */
ptr_station removeStation(ptr_station ptStations, uint32_t distance){
    ptr_station ptTemp;
    ptr_station ptTempPrevious;
    bool found = TRUE;

    ptTemp = ptStations;
    ptTempPrevious = NULL;
    while(ptTemp != NULL && ptTemp->distance != distance){
        ptTempPrevious = ptTemp;
        ptTemp = ptTemp->next;
    }

    if(ptTemp == NULL){
        found = FALSE;
    } else {
        if(ptTempPrevious == NULL){
            ptTemp->next->previous = ptTempPrevious;
            ptStations = ptTemp->next;
            free(ptTemp);
        } else if(ptTemp->next == NULL){
            ptTempPrevious->next = NULL;
            free(ptTemp);
        } else {
            ptTempPrevious->next = ptTemp->next;
            ptTemp->next->previous = ptTempPrevious;
            free(ptTemp);
        }
    }

    if(!found){
        printf("non demolita\n");
    } else {
        printf("demolita\n");
    }

    return(ptStations);
}

/**
 * Funzione necessaria per la rimozione di tutte le stazioni.
 * @param ptrStations è il puntatore all'inizio dell'autostrada.
 * @return il puntatore all'inizio della lista aggiornata.
 */
ptr_station destroyStations(ptr_station ptrStations){
    ptr_station ptTemp;

    while(ptrStations != NULL){
        ptTemp = ptrStations;
        ptrStations = ptrStations->next;
        free(ptTemp);
    }
    ptrStations = NULL;

    return(ptrStations);
}

/**
 * Aggiunta di un veicolo nel parco macchine della stazione di servizio. Se la stazione non esiste, non si aggiunge il veicolo.
 * @param ptStations è il puntatore all'inizio dell'autostrada.
 * @param distance è la distanza della stazione alla quale si deve aggiungere il veicolo.
 * @param vehicle è il veicolo da aggiungere.
 */
void addVehicle(ptr_station ptStations, uint32_t distance, uint32_t vehicle){
    ptr_station ptTemp;
    bool found = TRUE;
    bool addedVehicle;
    int i;

    ptTemp = ptStations;
    while(ptTemp != NULL && ptTemp->distance != distance){
        ptTemp = ptTemp->next;
    }

    if(ptTemp == NULL){
        found = FALSE;
    } else {
        i = 0;
        addedVehicle = FALSE;
        while(!addedVehicle && i < NUM_OF_CARS){
            if(ptTemp->vehiclesInStation[i] == 0){
                ptTemp->vehiclesInStation[i] = vehicle;
                sortVehicles(ptTemp->vehiclesInStation, 0, NUM_OF_CARS - 1);
                addedVehicle = TRUE;
            }
            i++;
        }
    }

    if(!found){
        printf("non aggiunta auto\n");
    } else {
        printf("aggiunta auto\n");
    }
}

/**
 * Rimozione di un veicolo dal parco macchine della stazione di servizio. Se la stazione non esiste o se nella stazione il veicolo non esiste non lo si rottama.
 * @param ptStations è il puntatore all'inizio dell'autostrada.
 * @param distance è la distanza della stazione dalla quale si deve rimuovere il veicolo.
 * @param vehicleToRemove è il veicolo da rimuovere.
 */
void removeVehicle(ptr_station ptStations, uint32_t distance, uint32_t vehicleToRemove){
    ptr_station ptTemp;
    bool found = TRUE;
    bool removedVehicle;
    int i;

    ptTemp = ptStations;
    while(ptTemp != NULL && ptTemp->distance != distance){
        ptTemp = ptTemp->next;
    }

    if(ptTemp == NULL){
        found = FALSE;
    } else {
        i = 0;
        removedVehicle = FALSE;
        while(!removedVehicle && i < NUM_OF_CARS){
            if(ptTemp->vehiclesInStation[i] == vehicleToRemove){
                ptTemp->vehiclesInStation[i] = 0;
                sortVehicles(ptTemp->vehiclesInStation, 0, NUM_OF_CARS - 1);
                removedVehicle = TRUE;
            }
            i++;
        }
    }

    if(!found || !removedVehicle){
        printf("non rottamata\n");
    } else {
        printf("rottamata\n");
    }
}

/**
 * Funzione che trova il numero di stazioni intermedie tra l'inizio e la fine del percorso.
 * @param ptrStations è il puntatore all'inizio dell'autostrada.
 * @param from è la stazione di partenza.
 * @param to è la stazione di arrivo.
 * @param direction è la direzione del percorso. FWD = da sinistra a destra, REV = da destra a sinistra.
 * @return il numero di stazioni intermedie.
 */
int findNumOfStations(ptr_station ptrStations, uint32_t from, uint32_t to, direction direction) {
    ptr_station ptTemp;
    ptr_station ptStart;
    ptr_station ptEnd;
    int numOfStations = 0;

    switch(direction){
        case FWD:
            ptTemp = ptrStations;
            while(ptTemp->distance != from){
                ptTemp = ptTemp->next;
            }
            ptStart = ptTemp;

            while(ptTemp->distance != to){
                ptTemp = ptTemp->next;
            }
            ptEnd = ptTemp;

            while(ptStart != ptEnd){
                numOfStations++;
                ptStart = ptStart->next;
            }
            break;
        case REV:
            ptTemp = ptrStations;
            while(ptTemp->distance != from){
                ptTemp = ptTemp->next;
            }
            ptStart = ptTemp;

            ptTemp = ptrStations;
            while(ptTemp->distance != to){
                ptTemp = ptTemp->next;
            }
            ptEnd = ptTemp;

            while(ptStart != ptEnd){
                numOfStations++;
                ptStart = ptStart->previous;
            }
            break;
        default:
            numOfStations = 0;
    }

    return (numOfStations - 1);
}


/*
 * =======================
 * Utility functions below
 * =======================
 */

/**
 * Quicksort per ordinare in ordine decrescente le automobili in una stazione.
 * @param array è l'array delle automobili da ordinare.
 * @param left è l'indice più a sinistra dell'array da ordinare.
 * @param right è l'indice più a destra dell'array da ordinare.
 */
void sortVehicles(uint32_t* array, int left, int right){
    if(left < right){
        int q = partition(array, left, right);
        sortVehicles(array, left, q - 1);
        sortVehicles(array, q + 1, right);
    }
}

/**
 * Funzione d'aiuto per quicksort. Permette di dividere l'array in sottoarray più piccoli.
 * @param array è l'array da dividere.
 * @param left è l'indice più a sinistra dell'array da dividere.
 * @param right è l'indice più a destra dell'array da dividere.
 * @return l'indice che delimita i sottoarray da ordinare tramite chiamate ricorsive di quicksort.
 */
int partition(uint32_t* array, int left, int right) {
    uint32_t pivot = array[right];
    int i = left - 1;
    for(int j = left; j <= right - 1; j++){
        if(array[j] > pivot){
            i++;
            swap(&array[i], &array[j]);
        }
    }

    swap(&array[i + 1], &array[right]);

    return i + 1;
}

/**
 * Funzione che permette di scambiare di posto due numeri.
 * @param a è il primo numero da scambiare.
 * @param b è il secondo numero da scambiare.
 */
void swap(uint32_t* a, uint32_t* b){
    uint32_t temp = *a;
    *a = *b;
    *b = temp;
}

/**
 * Funzione che azzera un array.
 * @param ptrArray è il puntatore all'array.
 * @param length è la lunghezza dell'array;
 */
void resetArray(bool *ptrArray, int length) {
    int i;
    for(i = 0; i < length; i++){
        ptrArray[i] = 0;
    }
}

/**
 * Funzione che ritorna la stazione alla distanza richiesta. La stazione deve esistere.
 * @param ptrStations è il puntatore all'inizio dell'autostrada.
 * @param distance è la distanza della stazione.
 * @return il puntatore alla stazione con tale distanza.
 */
ptr_station findStation(ptr_station ptrStations, uint32_t distance) {
    ptr_station ptrTemp;

    ptrTemp = ptrStations;
    while(ptrTemp->distance != distance){
        ptrTemp = ptrTemp->next;
    }

    return ptrTemp;
}