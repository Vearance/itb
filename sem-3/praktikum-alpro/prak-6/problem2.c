/* File: problem.c */
/* Implementasi fungsi untuk menyusun ulang linked list berdasarkan posisi ganjil dan genap */

#include "listlinier.h"
#include <stdio.h>
#include <stdlib.h>

void insertFirst(List *l, ElType val) {
/* KAMUS LOKAL */
    Address p;
/* ALGORITMA */
    p = newNode(val);
    if (p!=NULL) { /* alokasi berhasil */
        NEXT(p) = FIRST(*l);
        FIRST(*l) = p;
    }
}

ElType getElmt(List l, int idx) {
/* KAMUS LOKAL */
    int ctr;
    Address p;
/* ALGORITMA */
    ctr = 0;
    p = FIRST(l);
    while (ctr<idx) {
        ctr++;
        p = NEXT(p);
    }
    return INFO(p);
}

void deleteAt(List *l, int idx, ElType *val) {
/* KAMUS LOKAL */
    int ctr;
    Address p, loc;
/* ALGORITMA */
    if (idx==0) {
        deleteFirst(l, val);
    } else {
        ctr = 0;
        loc = FIRST(*l);
        while (ctr<idx-1) {
            ctr++;
            loc = NEXT(loc);
        }
        p = NEXT(loc);
        *val = INFO(p);
        NEXT(loc) = NEXT(p);
        free(p);
    }
}
void deleteFirst(List *l, ElType *val) {
/* KAMUS LOKAL */
    Address p;
/* ALGORITMA */
    p = FIRST(*l);
    *val = INFO(p);
    FIRST(*l) = NEXT(p);
    free(p);
}

/*
 * Fungsi untuk menyusun ulang elemen-elemen linked list sehingga
 * semua elemen pada posisi ganjil (1, 3, 5, ...) muncul lebih dulu,
 * diikuti oleh elemen pada posisi genap (2, 4, 6, ...).
 * Urutan relatif antar elemen ganjil maupun antar elemen genap
 * harus tetap dipertahankan.
 *
 * I.S.  : l terdefinisi, mungkin kosong
 * F.S.  : Elemen-elemen dalam list l diubah urutannya secara in-place
 *          tanpa alokasi/dealokasi node baru.
 *
 * Contoh:
 * Jika l adalah [1,2,3,4,5], maka setelah oddEvenLinkedList(&l), l menjadi [1,3,5,2,4]
 * Jika l adalah [10,20,30,40,50,60], maka hasilnya [10,30,50,20,40,60]
 * Jika l adalah [7], maka tetap [7]
 * Jika l adalah [], maka tetap []

*/
boolean isEmpty(List l) {
    return (FIRST(l) == NULL);
}
int length(List l) {
/* KAMUS LOKAL */
    int ctr;
    Address p;
/* ALGORITMA */
    ctr = 0;
    p = FIRST(l);
    while (p!=NULL) {
        ctr++;
        p = NEXT(p);
    }
    return ctr;
}
void oddEvenLinkedList(List *l) {
    Address oddPtr, evenPtr, evenHead;

    if (isEmpty(*l) || (*l)->next == NULL) {
        return;
    }

    oddPtr = (*l);
    evenHead = (*l)->next;
    evenPtr = evenHead;

    while (evenPtr != NULL && evenPtr->next != NULL) {
        oddPtr->next = evenPtr->next;
        oddPtr = oddPtr->next;

        evenPtr->next = oddPtr->next;
        evenPtr = evenPtr->next;
    }

    oddPtr->next = evenHead;
}

// void main () {
//     List l;
//     CreateList(&l);

//     insertFirst(&l, 1);
//     insertFirst(&l, 2);
//     insertFirst(&l, 3);
//     insertFirst(&l, 4);
//     insertFirst(&l, 5);
//     insertFirst(&l, 6);

//     displayList(l);
//     oddEvenLinkedList(&l);
//     displayList(l);
// }