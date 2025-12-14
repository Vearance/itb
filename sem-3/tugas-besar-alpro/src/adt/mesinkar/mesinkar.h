#ifndef MESINKAR_H
#define MESINKAR_H

#include "../boolean.h"

#include <stdio.h>
#define MARK ';'
#define BLANK ' '

extern char CC;
extern boolean EOP;

void START();
void STARTFILE(FILE *file);
void ADV();
void IgnoreBlank();

/* CSV-specific functions */
void STARTFILECSV(FILE *file);
void ADVCSV();
boolean IsEOF();

#endif
