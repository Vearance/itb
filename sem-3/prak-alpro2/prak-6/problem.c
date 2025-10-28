/* File: problem.c */
/* Implementasi fungsi untuk membalik linked list */

#include "listlinier.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Fungsi untuk membalik linked list secara in-place
 * I.S. l terdefinisi, mungkin kosong
 * F.S. Elemen-elemen dalam list l dibalik urutannya
 *
 * Contoh:
 * Jika l adalah [1,2,3,4,5], maka setelah reverseList(&l), l menjadi [5,4,3,2,1]
 * Jika l adalah [], maka setelah reverseList(&l), l tetap []
 * Jika l adalah [1], maka setelah reverseList(&l), l tetap [1]
 *
 * Hint: Gunakan 3 pointer (prev, current, next) untuk membalik arah pointer
 *       Jangan lupa update FIRST(l) di akhir!
 */
void reverseList(List *l) {
    /* KAMUS LOKAL */
    // TODO: Deklarasikan variabel yang diperlukan
    List lNew;
    Address pHelp, pFirst;


    /* ALGORITMA */
    // TODO: Implementasikan algoritma untuk membalik linked list
    if (!isEmpty(*l) || (length(*l) != 1)) {
    //     // keluar
    //     printf("haha");
    // }
    // else {
        CreateList(&lNew);

        pHelp = *l;
        pFirst = lNew;

        while (pHelp != NULL) {
            insertFirst(&lNew, pHelp->info);
            pHelp = pHelp->next;
        }

        // displayList(lNew);

        *l = lNew;

        // displayList(*l);

    }
}


// void main () {
//     List l;
//     CreateList(&l);

//     insertFirst(&l, 1);
//     insertFirst(&l, 2);
//     insertFirst(&l, 3);

//     displayList(l);
//     reverseList(&l);
//     displayList(l);
// }