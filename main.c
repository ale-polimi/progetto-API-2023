#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

/* Remove comment from this define for extra prints used for debug */
//#define DEBUG 1

/* Remove comment from this define for time measurement */
//#define TIME_DEBUG 1

#define FALSE 0
#define TRUE (!FALSE)
#define FWD 0
#define REV (!FWD)
#define NUM_OF_CARS 512
#define BUFF_LEN 8000 //Max caratteri ammissibili per 512 numeri a 32 bit + spazi + comando "aggiungi-stazione"

#define CACHE_LENGTH 4

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
    uint16_t numberOfCars;
    uint32_t *vehiclesInStation;
    int maxCapacityIndex;
    struct station *previous;
    struct station *next;
} t_station;

typedef t_station* ptr_station;
typedef int bool;
typedef int direction;

/*
 * CACHE MANAGEMENT
 */

ptr_station cachedStation[CACHE_LENGTH];
int lastAdded;

void resetCache(){
    for(int i = 0; i < CACHE_LENGTH; i++){
        cachedStation[i] = NULL;
    }
    lastAdded = 0;
}

void addToCache(ptr_station station){
    lastAdded = (lastAdded + 1) % CACHE_LENGTH;
    cachedStation[lastAdded] = station;
}

void removeFromCache(ptr_station station){
    for(int i = 0; i < CACHE_LENGTH; i++){
        if(cachedStation[i] != NULL && cachedStation[i] == station){
            cachedStation[i] = NULL;
            break;
        }
    }
}

int searchInCache(uint32_t distance){
    for(int i = 0; i < CACHE_LENGTH; i++){
        if(cachedStation[i] != NULL && cachedStation[i]->distance == distance){
            return i;
        }
    }

    return -1;
}

/*
 * END OF CACHE MANAGEMENT
 */

// Specification commands' functions
bool safe_fgets(char*);
ptr_station addStation(ptr_station, uint32_t, uint16_t, uint32_t*);
ptr_station removeStation(ptr_station, uint32_t);
void addVehicle(ptr_station, uint32_t, uint32_t);
void removeVehicle(ptr_station, uint32_t, uint32_t);

// Path finding
void dijkstra(ptr_station, ptr_station, ptr_station, int, int);

// Sorting functions
void sortVehicles(uint32_t*, int, int);
int partition(uint32_t*, int, int);
void swap(uint32_t*, uint32_t*);

// Utility functions
ptr_station findStation(ptr_station, uint32_t);
int findNumOfStations(ptr_station, uint32_t, uint32_t, direction);
ptr_station destroyStations(ptr_station);

int main() {
    ptr_station autostrada;
    char *inputBuffer = (char *) malloc(BUFF_LEN * sizeof(char));
    autostrada = NULL;
    direction direction;

    resetCache();

#ifdef TIME_DEBUG
gettimeofday(&start, NULL);
struct timeval stop, start;
#endif

    while(TRUE){
#ifdef DEBUG
printf("Cache: [");
for(int cache = 0; cache < CACHE_LENGTH - 1; cache++){
    printf("%u, ", ((cachedStation[cache] != NULL) ? cachedStation[cache]->distance : 0));
}
printf("%u]\n", ((cachedStation[CACHE_LENGTH - 1] != NULL) ? cachedStation[CACHE_LENGTH - 1]->distance : 0));
printf("lastAdded = %d\n", lastAdded);
#endif

        if(safe_fgets(inputBuffer)){
            if(inputBuffer[0] == 'a'){
                if(inputBuffer[9] == 's'){
                    /* Comando: aggiungi-stazione */

                    int i = 0;
                    uint32_t tempDistance = 0;
                    uint16_t tempNumOfCars = 0;
                    uint32_t *tempVehicles = NULL;
                    char* p = inputBuffer + 18;
                    while(p < inputBuffer+BUFF_LEN ) {
                        char *end;
                        uint32_t tempValue = strtol(p, &end, 10);
                        if (tempValue == 0L && end == p)  //docs also suggest checking errno value
                            break;

                        if(i == 0){
                            tempDistance = tempValue;
                        } else if(i == 1) {
                            tempNumOfCars = tempValue;
                            tempVehicles = malloc(tempNumOfCars*sizeof(uint32_t));
                        } else {
                            tempVehicles[i - 2] = tempValue;
                        }
                        i++;

                        p = end;
                    }

                    autostrada = addStation(autostrada, tempDistance, tempNumOfCars, tempVehicles);
                    free(tempVehicles);

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

                                if(ptTemp->vehiclesInStation != NULL){
                                    if(ptTemp->next->distance - ptTemp->distance <= ptTemp->vehiclesInStation[ptTemp->maxCapacityIndex]){
                                        printf("%u %u\n", ptTemp->distance, ptTemp->next->distance);
                                    } else {
                                        printf("nessun percorso\n");
                                    }
                                } else {
                                    printf("nessun percorso\n");
                                }
                            } else {

#ifdef DEBUG
printf("NumOfStations: %d\n", numOfStations + 2);
#endif

                                ptr_station startStation = findStation(autostrada, from);
                                ptr_station endStation = findStation(startStation, to);
                                dijkstra(autostrada, startStation, endStation, numOfStations + 2, FWD);
                            }
                            break;
                        case REV:
                            if(numOfStations == 0){
                                ptr_station ptTemp;

                                ptTemp = findStation(autostrada, from);

                                if(ptTemp->vehiclesInStation != NULL){
                                    if(ptTemp->distance - ptTemp->previous->distance <= ptTemp->vehiclesInStation[ptTemp->maxCapacityIndex]){
                                        printf("%u %u\n", ptTemp->distance, ptTemp->previous->distance);
                                    } else {
                                        printf("nessun percorso\n");
                                    }
                                } else {
                                    printf("nessun percorso\n");
                                }
                            } else {

#ifdef DEBUG
printf("NumOfStations: %d\n", numOfStations + 2);
#endif
                                ptr_station endStation = findStation(autostrada, to);
                                ptr_station startStation = findStation(endStation, from);
                                dijkstra(autostrada, startStation, endStation, numOfStations + 2, REV);
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
#ifdef TIME_DEBUG
gettimeofday(&stop, NULL);
printf("Execution time: %lu us\n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);
printf("Cache size: %d\n", CACHE_LENGTH);
#endif
    return 0;
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
 * Aggiunta in ordine crescente di una stazione all'autostrada. Se è già presente una stazione di servizio alla distanza specificata, non la si aggiunge.
 * @param ptStations è il puntatore all'inizio della lista.
 * @param distance è la distanza della nuova stazione dall'inizio dell'autostrada.
 * @param numOfCars è il numero di veicoli all'interno della stazione.
 * @param vehicles è il puntatore all'array contenente i veicoli della nuova stazione.
 * @return il puntatore all'inizio della lista aggiornata.
 */
ptr_station addStation(ptr_station ptStations, uint32_t distance, uint16_t numOfCars, uint32_t* vehicles){
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
        ptTempToAdd->numberOfCars = numOfCars;
        if(numOfCars == 0){
            ptTempToAdd->vehiclesInStation = NULL;
        } else {
            ptTempToAdd->vehiclesInStation = malloc(numOfCars*sizeof(uint32_t));
            for(i = 0; i < numOfCars; i++){
                ptTempToAdd->vehiclesInStation[i] = vehicles[i];
            }
        }

        if(numOfCars > 0){
            sortVehicles(ptTempToAdd->vehiclesInStation, 0, numOfCars - 1);
        }
        /* Dopo l'ordinamento il massimo si trova all'indice 0 */
        ptTempToAdd->maxCapacityIndex = 0;

        ptTempToAdd->previous = NULL;
        ptTempToAdd->next = NULL;

        if(ptStations == NULL){
            ptStations = ptTempToAdd;
            addToCache(ptStations);
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
                    addToCache(ptStations);
                } else if (ptTemp == NULL) {
                    /* Aggiungo in coda */
                    /* Collego il nodo precedente al nuovo nodo */
                    ptTempPrevious->next = ptTempToAdd;
                    /* Collego il nuovo nodo al nodo precedente */
                    ptTempToAdd->previous = ptTempPrevious;
                    /* Collego il nuovo nodo al successivo */
                    ptTempToAdd->next = ptTemp;
                    addToCache(ptTempPrevious->next);
                } else {
                    /* Collego il nodo precedente al nuovo nodo */
                    ptTempPrevious->next = ptTempToAdd;
                    /* Collego il nuovo nodo al nodo precedente */
                    ptTempToAdd->previous = ptTempPrevious;
                    /* Collego il nuovo nodo al successivo */
                    ptTempToAdd->next = ptTemp;
                    /* Collego il nodo successivo al nuovo nodo */
                    ptTemp->previous = ptTempToAdd;
                    addToCache(ptTempPrevious->next);
                }
            }
        }

        if(!alreadyPresent){
            // printf("aggiunta stazione\n");
            printf("aggiunta\n");
        } else {
            // printf("non aggiunta stazione\n");
            printf("non aggiunta\n");
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
        if(ptTempPrevious == NULL && ptTemp->next == NULL){
            /* Unico elemento della lista */
            ptStations = NULL;
        } else if(ptTempPrevious == NULL){
            /* Primo elemento, ma ce ne sono altri dopo */
            ptTemp->next->previous = ptTempPrevious;
            ptStations = ptTemp->next;
        } else if(ptTemp->next == NULL){
            /* Ultimo elemento */
            ptTempPrevious->next = NULL;
        } else {
            ptTempPrevious->next = ptTemp->next;
            ptTemp->next->previous = ptTempPrevious;
        }

        removeFromCache(ptTemp);
        free(ptTemp->vehiclesInStation);
        free(ptTemp);
    }

    if(!found){
        printf("non demolita\n");
    } else {
        printf("demolita\n");
    }

    return(ptStations);
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
    bool isSpaceAvailable = TRUE;

    int cacheIndex = searchInCache(distance);
    if(cacheIndex >= 0){
        ptTemp = cachedStation[cacheIndex];
    } else {
        ptTemp = ptStations;
        while(ptTemp != NULL && ptTemp->distance != distance){
            ptTemp = ptTemp->next;
        }
    }

    if(ptTemp == NULL){
        found = FALSE;
    } else {

        if(ptTemp->numberOfCars >= NUM_OF_CARS){
            isSpaceAvailable = FALSE;
        } else {
            ptTemp->vehiclesInStation = realloc(ptTemp->vehiclesInStation, (ptTemp->numberOfCars + 1)*sizeof(uint32_t));
            if(ptTemp->vehiclesInStation != NULL){
                ptTemp->vehiclesInStation[ptTemp->numberOfCars] = vehicle;
                /* Se il veicolo che aggiungo ha capacità maggiore del veicolo con capacità maggiore nell'array,
                 * diventa il nuovo veicolo con capacità maggiore.
                 */
                if(vehicle > ptTemp->vehiclesInStation[ptTemp->maxCapacityIndex]){
                    ptTemp->maxCapacityIndex = ptTemp->numberOfCars;
                }
                ptTemp->numberOfCars += 1;
            }
        }
    }

    if(!found || !isSpaceAvailable){
        // printf("non aggiunta auto\n");
        printf("non aggiunta\n");
    } else {
        // printf("aggiunta auto\n");
        printf("aggiunta\n");
    }

    addToCache(ptTemp);
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

    int cachedIndex = searchInCache(distance);
    if(cachedIndex >= 0){
        ptTemp = cachedStation[cachedIndex];
    } else {
        ptTemp = ptStations;
        while(ptTemp != NULL && ptTemp->distance != distance){
            ptTemp = ptTemp->next;
        }
    }

    if(ptTemp == NULL){
        found = FALSE;
    } else {
        if(ptTemp->numberOfCars < 1){
            found = FALSE;
        } else {
            i = 0;
            removedVehicle = FALSE;
            while(!removedVehicle && i < ptTemp->numberOfCars){
                if(ptTemp->vehiclesInStation[i] == vehicleToRemove){
                    ptTemp->vehiclesInStation[i] = 0;
                    sortVehicles(ptTemp->vehiclesInStation, 0, ptTemp->numberOfCars - 1);
                    if(ptTemp->numberOfCars == 1){
                        free(ptTemp->vehiclesInStation);
                    } else {
                        ptTemp->vehiclesInStation = realloc(ptTemp->vehiclesInStation, (ptTemp->numberOfCars - 1)*sizeof(uint32_t));
                    }
                    ptTemp->maxCapacityIndex = 0;
                    ptTemp->numberOfCars -= 1;
                    removedVehicle = TRUE;
                }
                i++;
            }

            if(ptTemp->numberOfCars == 0){
                //free(ptTemp->vehiclesInStation);
                ptTemp->vehiclesInStation = NULL;
            }
        }
    }

    if(!found || !removedVehicle){
        printf("non rottamata\n");
    } else {
        printf("rottamata\n");
    }

    addToCache(ptTemp);
}

/*
 * =========================================
 *      START OF PATHFINDING FUNCTIONS.
 * =========================================
 */

/**
 * Ricerca del percorso ottimo tra due stazioni di servizio dell'autostrada.
 * @param autostrada è il puntatore all'inizio dell'autostrada.
 * @param start è la stazione di partenza.
 * @param end è la stazione di arrivo.
 * @param numOfStations è il numero di stazioni tra la partenza e l'arrivo.
 * @param direction è la direzione di percorrenza dell'autostrada. Può essere {@code FWD} oppure {@code REV}.
 */
void dijkstra(ptr_station autostrada, ptr_station start, ptr_station end, int numOfStations, int direction) {
    ptr_station *stations = malloc(numOfStations * sizeof(ptr_station));
    bool *visited = malloc(numOfStations*sizeof(bool));
    int *parent = malloc(numOfStations*sizeof(int));
    int *queue = calloc(numOfStations, sizeof(int));
    uint32_t stationDistance;
    uint32_t vehicleCapacity;
    int vehicleIndex;

    switch(direction) {
        case FWD: {

            /* Init data structures */
            ptr_station ptTemp = start;
            for (int i = 0; i < numOfStations; i++) {
                stations[i] = ptTemp;
                stationDistance = ptTemp->distance;
                if(ptTemp->vehiclesInStation != NULL){
                    vehicleIndex = ptTemp->maxCapacityIndex;
                    vehicleCapacity = ptTemp->vehiclesInStation[vehicleIndex];
                } else {
                    vehicleCapacity = 0;
                }
                visited[i] = FALSE;
                parent[i] = -1;
                if (ptTemp != end) {
                    ptTemp = ptTemp->next;
                    if (stationDistance + vehicleCapacity < ptTemp->distance) {
                        visited[i] = TRUE;
                    }
                }
            }

            int head = 0;
            int tail = 0;
            queue[tail] = 0;
            tail++;
            visited[0] = TRUE;
            while (head != tail) {

#ifdef DEBUG
printf("QueueFWD= [");
for(int index = 0; index < numOfStations - 1; index++){
    printf("%d, ", queue[index]);
}
printf("%d]\n", queue[numOfStations - 1]);
#endif

                int u = queue[head];
                head++;
                for (int i = u + 1; i < numOfStations; i++) {
                    if (visited[i] == FALSE) {
                        stationDistance = stations[u]->distance;
                        if(stations[u]->vehiclesInStation != NULL){
                            vehicleIndex = stations[u]->maxCapacityIndex;
                            vehicleCapacity = stations[u]->vehiclesInStation[vehicleIndex];
                        } else {
                            vehicleCapacity = 0;
                        }
                        if (stationDistance + vehicleCapacity >= stations[i]->distance) {
                            queue[tail] = i;
                            tail++;
                            visited[i] = TRUE;
                            parent[i] = u;
                        } else {
                            break;
                        }
                    }
                }
            }

            if (visited[numOfStations - 1] == 0) {
                printf("nessun percorso\n");
            } else {
                uint32_t *path = malloc(numOfStations * sizeof(int));
                int i = numOfStations - 1;
                int j = 0;
                while (i != -1) {
                    path[j] = stations[i]->distance;
                    i = parent[i];
                    j++;
                }

                for (int k = j - 1; k > 0; k--) {
                    printf("%u ", path[k]);
                }
                printf("%u\n", path[0]);
                free(path);
            }
            break;
        }
        case REV: {

            /* Init data structures */
            ptr_station ptTemp = start;
            for (int i = 0; i < numOfStations; i++) {
                stations[numOfStations - 1 - i] = ptTemp;
                stationDistance = ptTemp->distance;
                if(ptTemp->vehiclesInStation != NULL){
                    vehicleIndex = ptTemp->maxCapacityIndex;
                    vehicleCapacity = ptTemp->vehiclesInStation[vehicleIndex];
                } else {
                    vehicleCapacity = 0;
                }
                visited[i] = FALSE;
                parent[i] = -1;
                if (ptTemp != end) {
                    ptTemp = ptTemp->previous;
                    if (stationDistance > ptTemp->distance + vehicleCapacity) {
                        visited[i] = TRUE;
                    }
                }
            }

            int head = 0;
            int tail = 0;
            queue[tail] = 0;
            tail++;
            visited[0] = 1;
            while (head != tail) {

#ifdef DEBUG
printf("QueueREV= [");
for(int index = 0; index < numOfStations - 1; index++){
    printf("%d, ", queue[index]);
}
printf("%d]\n", queue[numOfStations - 1]);
#endif

                int u = queue[head];
                head++;
                for (int i = u + 1; i < numOfStations; i++) {
                    if (visited[i] == FALSE) {
                        stationDistance = stations[i]->distance;
                        if(stations[i]->vehiclesInStation != NULL){
                            vehicleIndex = stations[i]->maxCapacityIndex;
                            vehicleCapacity = stations[i]->vehiclesInStation[vehicleIndex];
                        } else {
                            vehicleCapacity = 0;
                        }
                        if (stationDistance <= stations[u]->distance + vehicleCapacity) {
                            queue[tail] = i;
                            tail++;
                            visited[i] = TRUE;
                            parent[i] = u;
                        }
                    }
                }
            }

            if (visited[numOfStations - 1] == 0) {
                printf("nessun percorso\n");
            } else {
                int i = numOfStations - 1;

                while (parent[i] != -1) {
                    printf("%u ", stations[i]->distance);
                    i = parent[i];
                }
                printf("%u\n", stations[0]->distance);
            }
            break;
        }
    }

    free(stations);
    free(visited);
    free(parent);
    free(queue);
}

/*
 * =========================================
 *      END OF PATHFINDING FUNCTIONS.
 * =========================================
 */

/*
 * =======================
 * UTILITY FUNCTIONS BELOW
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
