#ifndef MESINKATA_H
#define MESINKATA_H

#include "../mesinkar/mesinkar.h"
#include "../boolean.h"

#define NMax 2048

typedef struct {
    char TabKata[NMax+1];
    int Length;
} Kata;

extern Kata CKata;
extern boolean EndKata;

void IgnoreBlank();
void STARTKATA();
void STARTKATAFILE(FILE *file);
void ADVKATA();
void SalinKata();
void SalinKataQuoted();

/* Read content with spaces until MARK (;) */
void STARTKATALINE();
void SalinKataLine();


void MakeKataEmpty(Kata *K);
boolean IsKataEmpty(Kata K);
int KataLength(Kata K);
char* KataToString(Kata K);
Kata StringToKata(char* str);
boolean IsKataEqual(Kata K1, Kata K2);
int KataToInt(Kata K);
boolean IsKataInt(Kata K);

/* CSV Parsing Functions */
void STARTKATACSV(FILE *file);
void ADVKATACSV();
void SalinKataCSV();
boolean EndLine();
void SkipLine();

#endif
