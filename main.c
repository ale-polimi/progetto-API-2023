#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define FALSE 0
#define TRUE (!FALSE)
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

ptr_station addStation(ptr_station, uint32_t, uint32_t*);
ptr_station removeStation(ptr_station, uint32_t);
ptr_station destroyStations(ptr_station);
void addVehicle(ptr_station, uint32_t, uint32_t);
void removeVehicle(ptr_station, uint32_t, uint32_t);
void printStations(ptr_station);

void safe_fgets(char*);

// Sorting functions.
void sortVehicles(uint32_t*, int, int);
int partition(uint32_t*, int, int);
void swap(uint32_t*, uint32_t*);

int main() {
    ptr_station autostrada;
    char *inputBuffer = (char *) malloc(BUFF_LEN * sizeof(char));
    autostrada = NULL;

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

    /*
    uint32_t veicoli1[NUM_OF_CARS] = { 0 };
    uint32_t veicoli2[NUM_OF_CARS] = { 0 };
    uint32_t veicoli3[NUM_OF_CARS] = { 0 };
    uint32_t veicoli4[NUM_OF_CARS] = { 0 };

    veicoli1[0] = 10;
    veicoli1[1] = 3;
    veicoli1[2] = 4502;
    veicoli1[3] = 34;

    veicoli2[0] = 140;
    veicoli2[1] = 32;
    veicoli2[2] = 4;
    veicoli2[3] = 7342;
    veicoli2[4] = 23;

    veicoli3[0] = 12;
    veicoli4[0] = 1234;

    autostrada = addStation(autostrada, 10, veicoli1);
    autostrada = addStation(autostrada, 2, veicoli2);
    autostrada = addStation(autostrada, 11, veicoli3);
    //autostrada = addStation(autostrada, 11, veicoli4);
    //autostrada = addStation(autostrada, 20, veicoli4);

    //autostrada = removeStation(autostrada, 12);
    addVehicle(autostrada, 10, 10);
    //printStations(autostrada);
    addVehicle(autostrada, 3, 123);
    addVehicle(autostrada, 20, 2);
    printStations(autostrada);
    removeVehicle(autostrada, 10, 10);
    printStations(autostrada);

    return 0;
     */
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