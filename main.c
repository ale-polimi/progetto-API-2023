#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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

ptr_station addStation(ptr_station, uint32_t, uint32_t*);
ptr_station removeStation(uint32_t);
void addVehicle(uint32_t, uint32_t);
void removeVehicle(uint32_t, uint32_t);
void printStations(ptr_station);


int main() {
    ptr_station autostrada;

    autostrada = NULL;

    uint32_t veicoli1[512] = { 0 };
    uint32_t veicoli2[512] = { 0 };
    uint32_t veicoli3[512] = { 0 };
    int i;

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

    autostrada = addStation(autostrada, 10, veicoli1);
    printStations(autostrada);

    autostrada = addStation(autostrada, 2, veicoli2);
    printStations(autostrada);

    autostrada = addStation(autostrada, 5, veicoli3);
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
 * Aggiunta in ordine crescente di una stazione all'autostrada.
 * @param ptStations è il puntatore all'inizio della lista.
 * @param distance è la distanza della nuova stazione dall'inizio dell'autostrada.
 * @param vehicles è il puntatore all'array contenente i veicoli della nuova stazione.
 * @return il puntatore al'inizio della lista aggiornata.
 */
ptr_station addStation(ptr_station ptStations, uint32_t distance, uint32_t* vehicles){
    ptr_station ptTempToAdd;
    ptr_station ptTemp;
    ptr_station ptTempPrevious;
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

            if(ptTempPrevious == NULL){
                ptStations->previous = ptTempToAdd;
                ptTempToAdd->next = ptStations;
                ptStations = ptTempToAdd;
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

    return(ptStations);
}
