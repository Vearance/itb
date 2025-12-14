#include "mesinkata.h"
#include <stdio.h>
#include <stdlib.h>

static char* my_strncpy(char *dest, const char *src, int n) {
    int i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

static int my_strncmp(const char *s1, const char *s2, int n) {
    for (int i = 0; i < n; i++) {
        if (s1[i] != s2[i]) {
            return (unsigned char)s1[i] - (unsigned char)s2[i];
        }
        if (s1[i] == '\0') {
            return 0;
        }
    }
    return 0;
}

static int my_isdigit(char c) {
    return c >= '0' && c <= '9';
}

Kata CKata;
boolean EndKata;

void STARTKATA() {
    START();
    IgnoreBlank();
    if (EOP) {
        EndKata = true;
    } else {
        EndKata = false;
        SalinKata();
    }
}

void STARTKATAFILE(FILE *file) {
    STARTFILE(file);
    IgnoreBlank();
    if (EOP) {
        EndKata = true;
    } else {
        EndKata = false;
        SalinKata();
    }
}

void ADVKATA() {
    IgnoreBlank();
    if (EOP) {
        EndKata = true;
    } else {
        SalinKata();
        IgnoreBlank();
    }
}

void SalinKata() {
    int i = 0;
    MakeKataEmpty(&CKata);
    
    
    if (CC == '"') {
        ADV(); 
        while (CC != '"' && !EOP && i < NMax) {
            if (CC == '\\' && !EOP) {
                ADV();
                if (CC == '"') {
                    CKata.TabKata[i] = '"';
                } else if (CC == '\\') {
                    CKata.TabKata[i] = '\\';
                } else {
                    CKata.TabKata[i] = '\\';
                    i++;
                    if (i < NMax) {
                        CKata.TabKata[i] = CC;
                    }
                }
            } else {
                CKata.TabKata[i] = CC;
            }
            i++;
            ADV();
        }
        if (CC == '"') {
            ADV(); 
        }
    } else {
        
        while (CC != BLANK && CC != MARK && !EOP && i < NMax) {
            CKata.TabKata[i] = CC;
            i++;
            ADV();
        }
    }
    
    CKata.Length = i;
    CKata.TabKata[i] = '\0';
}

void SalinKataQuoted() {
    int i = 0;
    MakeKataEmpty(&CKata);
    
    
    while (CC != '"' && CC != MARK && !EOP && i < NMax) {
        CKata.TabKata[i] = CC;
        i++;
        ADV();
    }
    
    CKata.Length = i;
    CKata.TabKata[i] = '\0';
}

/* Read content with spaces until MARK (;) */
void STARTKATALINE() {
    START();
    IgnoreBlank();
    if (EOP) {
        EndKata = true;
    } else {
        EndKata = false;
        SalinKataLine();
    }
}

void SalinKataLine() {
    int i = 0;
    MakeKataEmpty(&CKata);
    
    /* Read everything until MARK (;) */
    while (!EOP && i < NMax) {
        CKata.TabKata[i] = CC;
        i++;
        ADV();
    }
    
    /* Trim trailing whitespace */
    while (i > 0 && (CKata.TabKata[i-1] == ' ' || CKata.TabKata[i-1] == '\t' || 
                     CKata.TabKata[i-1] == '\n' || CKata.TabKata[i-1] == '\r')) {
        i--;
    }
    
    CKata.Length = i;
    CKata.TabKata[i] = '\0';
}

void MakeKataEmpty(Kata *K) {
    K->Length = 0;
    K->TabKata[0] = '\0';
}

boolean IsKataEmpty(Kata K) {
    return K.Length == 0;
}

int KataLength(Kata K) {
    return K.Length;
}

char* KataToString(Kata K) {
    static char str[NMax+1];
    my_strncpy(str, K.TabKata, NMax);
    str[NMax] = '\0';
    return str;
}

Kata StringToKata(char* str) {
    Kata K;
    int i = 0;
    MakeKataEmpty(&K);
    
    while (str[i] != '\0' && i < NMax) {
        K.TabKata[i] = str[i];
        i++;
    }
    K.Length = i;
    K.TabKata[i] = '\0';
    
    return K;
}

boolean IsKataEqual(Kata K1, Kata K2) {
    if (K1.Length != K2.Length) {
        return false;
    }
    return my_strncmp(K1.TabKata, K2.TabKata, K1.Length) == 0;
}

int KataToInt(Kata K) {
    char str[NMax+1];
    my_strncpy(str, K.TabKata, K.Length);
    str[K.Length] = '\0';
    return atoi(str);
}

boolean IsKataInt(Kata K) {
    if (K.Length == 0) {
        return false;
    }
    
    int i = 0;
    
    if (K.TabKata[0] == '-' && K.Length > 1) {
        i = 1;
    }
    
    for (; i < K.Length; i++) {
        if (!my_isdigit(K.TabKata[i])) {
            return false;
        }
    }
    return true;
}

/* ============================================
   CSV Parsing Functions
   ============================================ */

static boolean EndLineCSV = false;

void STARTKATACSV(FILE *file) {
    STARTFILECSV(file);
    EndLineCSV = false;
    if (EOP) {
        EndKata = true;
    } else {
        EndKata = false;
    }
}

boolean EndLine() {
    return EndLineCSV;
}

void SkipLine() {
    while (!EOP && CC != '\n') {
        ADVCSV();
    }
    if (CC == '\n') {
        ADVCSV();
    }
    EndLineCSV = false;
}

void ADVKATACSV() {
    if (EOP) {
        EndKata = true;
        return;
    }
    if (EndLineCSV) {
        EndLineCSV = false;
    }
    SalinKataCSV();
}

void SalinKataCSV() {
    int i = 0;
    MakeKataEmpty(&CKata);
    
    /* Skip leading whitespace (but not newlines) */
    while ((CC == ' ' || CC == '\t') && !EOP) {
        ADVCSV();
    }
    
    /* Check for end of line */
    if (CC == '\n' || CC == '\r') {
        EndLineCSV = true;
        while ((CC == '\n' || CC == '\r') && !EOP) {
            ADVCSV();
        }
        return;
    }
    
    /* Check if field is quoted */
    if (CC == '"') {
        ADVCSV(); /* Skip opening quote */
        while (!EOP && i < NMax) {
            if (CC == '"') {
                ADVCSV();
                if (CC == '"') {
                    /* Escaped quote ("") -> single quote */
                    CKata.TabKata[i++] = '"';
                    ADVCSV();
                } else {
                    /* End of quoted field */
                    break;
                }
            } else {
                CKata.TabKata[i++] = CC;
                ADVCSV();
            }
        }
    } else {
        /* Unquoted field - read until comma or newline */
        while (CC != ',' && CC != '\n' && CC != '\r' && !EOP && i < NMax) {
            CKata.TabKata[i++] = CC;
            ADVCSV();
        }
    }
    
    CKata.Length = i;
    CKata.TabKata[i] = '\0';
    
    /* Skip comma separator */
    if (CC == ',') {
        ADVCSV();
    } else if (CC == '\n' || CC == '\r') {
        EndLineCSV = true;
        while ((CC == '\n' || CC == '\r') && !EOP) {
            ADVCSV();
        }
    }
}
