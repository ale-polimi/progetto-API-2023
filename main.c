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

int minDistance(uint32_t *, bool *, int);
void printPath(int *, int, uint32_t *);
void dijkstra(uint32_t **, int, int, uint32_t *, int);

ptr_station addStation(ptr_station, uint32_t, uint32_t*);
ptr_station removeStation(ptr_station, uint32_t);
ptr_station destroyStations(ptr_station);
void addVehicle(ptr_station, uint32_t, uint32_t);
void removeVehicle(ptr_station, uint32_t, uint32_t);

int findNumOfStations(ptr_station, uint32_t, uint32_t, direction);

void printStations(ptr_station);

bool safe_fgets(char*);

// Sorting functions.
void sortVehicles(uint32_t*, int, int);
int partition(uint32_t*, int, int);
void swap(uint32_t*, uint32_t*);

ptr_station findStation(ptr_station, uint32_t);
void resetArray(bool *, int);

void createGraphFwd(ptr_station, ptr_station, int, uint32_t *, uint32_t**);
void createGraphRev(ptr_station, ptr_station, int, uint32_t *, uint32_t**);
int indexOf(uint32_t *, int, uint32_t);

void printMatrix(uint32_t **, int);

int main() {
    ptr_station autostrada;
    char *inputBuffer = (char *) malloc(BUFF_LEN * sizeof(char));
    autostrada = NULL;
    direction direction;

    while(TRUE){
        if(safe_fgets(inputBuffer)){
            if(inputBuffer[0] == 'a'){
                if(inputBuffer[9] == 's'){
                    /* Comando: aggiungi-stazione */

                    int i = 0;
                    uint32_t tempDistance = 0;
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
                    uint32_t tempDistance = 0;
                    uint32_t tempVehicle = 0;
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

                uint32_t tempDistance = 0;
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
                uint32_t tempDistance = 0;
                uint32_t tempVehicle = 0;
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
                uint32_t from = 0;
                uint32_t to = 0;
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
                                createGraphFwd(startStation, endStation, numOfStations + 2, lut, adjacencyMatrix);
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
                                    ptrTemp = ptrTemp->previous;
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
                                createGraphRev(startStation, endStation, numOfStations + 2, lut, adjacencyMatrix);
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
                    }
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
 *      START OF PATHFINDING FUNCTIONS.
 * =========================================
 */

/**
 * Funzione per trovare la distanza minima da un nodo al successivo.
 * @param dist è l'array delle distanze tra i nodi.
 * @param visited è l'array che indica se i nodi sono stati visitati.
 * @param numOfStations è il numero di stazioni tra la stazione di inizio e quella di fine.
 * @return
 */
int minDistance(uint32_t *dist, bool *visited, int numOfStations) {
    int min = INT_MAX;
    int min_index = 0;

    for (int v = 0; v < numOfStations; v++) {
        if (!visited[v] && dist[v] <= min) {
            min = dist[v];
            min_index = v;
        }
    }

    return min_index;
}

/**
 * Funzione per stampare il percorso quando è stato trovato.
 * @param parent è il puntatore all'array contenente il parente del nodo i-esimo.
 * @param j è l'indice da cui partire.
 * @param lut è il puntatore alla LUT.
 */
void printPath(int *parent, int j, uint32_t *lut) {

    if (parent[j] == -1){
        return;
    }

    printPath(parent, parent[j], lut);
    printf("%d ", lut[j]);
}

/**
 * Algoritmo di Dijkstra per trovare il miglior percorso sul grafo.
 * @param graph è il grafo salvato come matrice di adiacenza.
 * @param src è il nodo di partenza.
 * @param dest è il nodo di destinazione.
 * @param lut è la LUT che trasforma le distanze in valori da 0 a numOfStations.
 * @param numOfStations è il numero di stazioni tra la partenza e la destinazione.
 */
void dijkstra(uint32_t **graph, int src, int dest, uint32_t *lut, int numOfStations) {
    uint32_t *dist = malloc(numOfStations*sizeof(uint32_t));
    bool *visited = malloc(numOfStations*sizeof(bool));
    int *parent = malloc(numOfStations*sizeof(int));

    /* Init data structures */
    for (int i = 0; i < numOfStations; i++) {
        dist[i] = UINT32_MAX;
        visited[i] = FALSE;
        parent[i] = -1;
    }

    /* Distance of source from source is always 0 */
    dist[src] = 0;

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
    free(parent);
}

/*
 * =========================================
 *      END OF PATHFINDING FUNCTIONS.
 * =========================================
 */

/**
 * Funzione che trova l'indice della stazione all'interno della LUT (Look-Up Table).
 * @param lut è il puntatore alla LUT.
 * @param length è la lunghezza della LUT.
 * @param distance è la distanza (ID della stazione) a cui si trova la stazione da cercare.
 * @return l'indice della LUT contenente tale stazione.
 */
int indexOf(uint32_t *lut, int length, uint32_t distance) {
    for(int i = 0; i < length; i++){
        if(lut[i] == distance){
            return i;
        }
    }

    return -1;
}

/**
 * Funzione utilizzata per creare un grafo (in direzione dall'inizio alla fine dell'autostrada).
 * @param departure è la stazione di partenza.
 * @param destination è la stazione di arrivo.
 * @param numOfStations è il numero di stazioni tra la stazione di partenza e quella di arrivo.
 * @param lut è il puntatore alla LUT.
 * @param adjacencyMatrix è il grafo, memorizzato come matrice di adiacenza.
 */
void createGraphFwd(ptr_station departure, ptr_station destination, int numOfStations, uint32_t * lut, uint32_t **adjacencyMatrix){

    if(departure == destination){
        return;
    }

    ptr_station ptrTemp = departure;
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
    createGraphFwd(departure->next, destination, numOfStations, lut, adjacencyMatrix);
}

/**
 * Funzione utilizzata per creare un grafo (in direzione dalla fine all'inizio dell'autostrada).
 * @param departure è la stazione di partenza.
 * @param destination è la stazione di arrivo.
 * @param numOfStations è il numero di stazioni tra la stazione di partenza e quella di arrivo.
 * @param lut è il puntatore alla LUT.
 * @param adjacencyMatrix è il grafo, memorizzato come matrice di adiacenza.
 */
void createGraphRev(ptr_station departure, ptr_station destination, int numOfStations, uint32_t * lut, uint32_t **adjacencyMatrix){

    if(departure == destination){
        return;
    }

    ptr_station ptrTemp = departure;
    while(ptrTemp != destination){
        if(ptrTemp != departure){
            if(departure->distance - ptrTemp->distance <= departure->vehiclesInStation[0]){
                adjacencyMatrix[indexOf(lut, numOfStations, departure->distance)][indexOf(lut, numOfStations, ptrTemp->distance)] = ptrTemp->distance - destination->distance;
            }
        }
        if(ptrTemp->previous == destination){
            if(departure->distance - ptrTemp->previous->distance <= departure->vehiclesInStation[0]){
                adjacencyMatrix[indexOf(lut, numOfStations, departure->distance)][indexOf(lut, numOfStations, ptrTemp->previous->distance)] = 1;
            }
        }
        ptrTemp = ptrTemp->previous;
    }
    createGraphRev(departure->previous, destination, numOfStations, lut, adjacencyMatrix);
}

/**
 * Funzione per ricevere in modo sicuro l'input.
 * @param s è il buffer per lo stream di input.
 */
bool safe_fgets(char *s){
    if(fgets(s, BUFF_LEN, stdin) == NULL){
        return FALSE;
    } else {
        return TRUE;
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
    bool removedVehicle = FALSE;
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