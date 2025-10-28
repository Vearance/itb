#include "listlinier.h"
#include <stdio.h>
#include <stdlib.h>

void swap(int *a, int *b){
   int c;
   c = *a;
   *a = *b;
   *b = c;
}

/**
 * Fungsi yang melakukan pengurutan dengan urutan menaik jika asc = true
 * dan menurun jika asc = false
 * 
 * I.S. List l terdefinisi dan berisi setidaknya 1 elemen
 * F.S. List l terurut secara urutan tertentu
 * 
 * Contoh: 
 * - Untuk l = [1, 5, 2, 3, 3], maka setelah sort(&l, true), l menjadi [1, 2, 3, 3, 5]
 */

void sort(List *l, boolean asc) {
    List lNew;
    Address pHelp, pFirst;
    int ukuran;


    if (!isEmpty(*l) || (length(*l) != 1)) {
        CreateList(&lNew);
        pHelp = *l;
        pFirst = lNew;
        ukuran = length(*l);

        if(asc == true) {
            for(int i = 0; i < ukuran; i++) {
                for(int j = 0; j < i; j++) {
                    if(getElmt(*l, i) < getElmt(*l, j)) {
                        ElType c = getElmt(*l, i);
                        setElmt(l, i, getElmt(*l, j));
                        setElmt(l, j, c);
                    }
                }
            }
        }
        else {
            for(int i = 0; i < ukuran; i++) {
                for(int j = 0; j < i; j++) {
                    if(getElmt(*l, i) > getElmt(*l, j)) {
                        ElType c = getElmt(*l, i);
                        setElmt(l, i, getElmt(*l, j));
                        setElmt(l, j, c);
                    }
                }
            }

        }
    }



}


// void main () {
//     List l;
//     CreateList(&l);

//     insertFirst(&l, 3);
//     insertFirst(&l, 2);
//     insertFirst(&l, 1);

//     displayList(l);
//     sort(&l, false);
//     displayList(l);
// }