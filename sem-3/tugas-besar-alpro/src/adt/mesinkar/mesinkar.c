#include "mesinkar.h"
#include <stdio.h>

char CC;
boolean EOP;
static FILE *pita;
static int retval;

void START() {
    pita = stdin;
    ADV();
}

void STARTFILE(FILE *file) {
    pita = file;
    ADV();
}

void ADV() {
    retval = fscanf(pita, "%c", &CC);
    if (retval == EOF) {
        EOP = true;
        CC = MARK;
    } else {
        EOP = (CC == MARK);
    }
    if (EOP && pita != stdin) {
        fclose(pita);
    }
}

void IgnoreBlank() {
    while ((CC == BLANK || CC == '\n' || CC == '\r' || CC == '\t') && !EOP) {
        ADV();
    }
}

/* CSV-specific functions - does not auto-close file and EOP only on EOF */
void STARTFILECSV(FILE *file) {
    pita = file;
    EOP = false;
    ADVCSV();
}

void ADVCSV() {
    retval = fscanf(pita, "%c", &CC);
    if (retval == EOF) {
        EOP = true;
        CC = '\0';
    } else {
        EOP = false;
    }
}

boolean IsEOF() {
    return EOP;
}
