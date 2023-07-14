#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define FALSE 0
#define TRUE (!FALSE)

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
    uint32_t vehiclesInStation[512];
    struct station *previous;
    struct station *next;
} t_station;

typedef t_station* ptr_station;

typedef int bool;

ptr_station addStation(ptr_station, uint32_t, uint32_t*);
ptr_station removeStation(ptr_station, uint32_t);
void addVehicle(ptr_station, uint32_t, uint32_t);
void removeVehicle(ptr_station, uint32_t, uint32_t);
void printStations(ptr_station);


int main() {
    ptr_station autostrada;

    autostrada = NULL;

    uint32_t veicoli1[512] = { 0 };
    uint32_t veicoli2[512] = { 0 };
    uint32_t veicoli3[512] = { 0 };
    uint32_t veicoli4[512] = { 0 };

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
    removeVehicle(autostrada, 10, 12);
    printStations(autostrada);

    return 0;
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
        for(int i = 0; i < 512; i++){
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
        for(i = 0; i < 512; i++){
            ptTempToAdd->vehiclesInStation[i] = vehicles[i];
        }
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
        while(!addedVehicle && i < 512){
            if(ptTemp->vehiclesInStation[i] == 0){
                ptTemp->vehiclesInStation[i] = vehicle;
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
        while(!removedVehicle && i < 512){
            if(ptTemp->vehiclesInStation[i] == vehicleToRemove){
                ptTemp->vehiclesInStation[i] = 0;
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