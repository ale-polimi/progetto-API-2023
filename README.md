# Prova Finale di Algoritmi e Principi dell'Informatica - A.A. 2022-2023

[![License: GPL v2](https://img.shields.io/badge/License-GPL_v2-blue.svg)](https://github.com/ale-polimi/progetto-API-2023/blob/master/LICENSE)

Prova finale di algoritmi e principi dell'informatica per l'anno accademico 2022-2023.

Docente: Martineghi Davide

**Valutazione**: 24/30

## Obiettivo del progetto

Realizzare un programma in C per la ricerca del percorso ottimo tra stazioni di servizio di un'autostrada.

Il programma non solo deve produrre un output corretto, ma deve rispettare dei vincoli di memoria e tempo CPU come in tabella:

| Valutazione | Memoria | Tempo |       Esito        |
|-------------|--------:|------:|:------------------:|
| 18          | 128 MiB | 19 s  | :white_check_mark: |
| 21          | 118 MiB | 15 s  | :white_check_mark: |
| 24          | 108 MiB | 10 s  | :white_check_mark: |
| 27          |  98 MiB |  6 s  |        :x:         |
| 30          |  88 MiB |  4 s  |        :x:         |
| 30L         |  78 MiB |  1 s  |        :x:         |

Al fine di rispettare i limiti di tempo è stato necessario creare una cache contenente i puntatori alle ultime stazioni modificate o accedute. Il grafico seguente rappresenta il tempo di esecuzione del programma con input il file `open_111.txt` al variare della grandezza della cache:

```mermaid
xychart-beta;
	x-axis "Dimensione cache" [1, 2, 3, 4, 5]
	y-axis "Tempo di esecuzione [s]" 40 --> 45
	bar [44, 44, 42, 41, 44]
	line [44, 44, 42, 41, 44]
```

> [!NOTE]
> I tempi di esecuzione sono molto alti siccome il programma è stato eseguito in una macchina virtuale per questo test.

Il mio progetto prendendo i dati dal verificatore ha i seguenti utilizzi di memoria e tempo:

- Memoria: ~6,1 MiB
- Tempo: ~6,3 s

Questi risultati possono variare a seconda della potenza di calcolo della macchina.

La specifica completa del progetto è disponibile [qui](https://github.com/ale-polimi/progetto-API-2023/blob/master/documents/Specifica_2022-2023.pdf).

I test sono disponibili [qui](https://github.com/ale-polimi/progetto-API-2023/blob/master/documents/open_tests).

## Strumenti utilizzati

| Descrizione         | Strumento                                  |
|---------------------|--------------------------------------------|
| IDE                 | [CLion](https://www.jetbrains.com/clion/)  |
| Compilatore         | [gcc](https://gcc.gnu.org/)                |
| Misurazione memoria | [Valgrind - Massif](https://valgrind.org/) |
| Sistema operativo   | Windows 10 e Debian 11                     |

## Copyright e licenza

Il progetto è distribuito sotto [licenza GPL v2](https://github.com/ale-polimi/progetto-API-2023/blob/master/LICENSE), si applicano le limitazioni descritte in tale licenza.