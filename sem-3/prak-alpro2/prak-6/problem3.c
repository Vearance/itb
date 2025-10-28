#include "listlinier.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Fungsi untuk menggabungkan nilai-nilai di antara dua node bernilai 0
 * menjadi satu node dengan nilai hasil penjumlahan, dan menghapus seluruh node 0.
 *
 * I.S.  l terdefinisi, berisi minimal tiga elemen, dengan ketentuan:
 *      - Elemen pertama dan terakhir bernilai 0
 *      - Tidak ada dua node 0 yang bersebelahan
 *
 * F.S.  Setiap rentang elemen di antara dua 0 dijumlahkan menjadi satu node baru,
 *       dan list hasil tidak mengandung nilai 0 sama sekali.
 *
 * Contoh:
 * Jika l = [0,3,1,0,4,5,2,0], maka setelah mergeList(&l), l menjadi [4,11]
 * Jika l = [0,1,0,3,0,2,2,0], maka setelah mergeList(&l), l menjadi [1,3,4]
 *
 * Hint:
 * - Gunakan variabel untuk menampung hasil penjumlahan sementara
 * - Jangan lupa update FIRST(l) di akhir!
 */
void mergeList(List *l) {
    List lNew;
    Address pHelp, pFirst;

    if (!isEmpty(*l) || (length(*l) != 1)) {
        CreateList(&lNew);

        pHelp = *l;
        pFirst = lNew;
        ElType tambah = 0;
        boolean modeTambah = false;
        // misal 0 pertama, mode tambahnya false
        // trus ketemu 0 pertama, mode tambah jadi true, skip ke next


        while (pHelp != NULL) {
            if (pHelp->info == 0 && modeTambah == false) {
                modeTambah = true;
                pHelp = pHelp->next;
            }
            else if (pHelp->info == 0 && modeTambah == true) {
                // tempat masukin ke list
                insertLast(&lNew, tambah);

                modeTambah = true;
                tambah = 0; //reset
                pHelp = pHelp->next;
            }
            else if (pHelp->info != 0) {
                tambah += pHelp->info;
                pHelp = pHelp->next;
            } 
        }

        // displayList(lNew);

        *l = lNew;

        // displayList(*l);

    }

}

// void main () {
//     List l;
//     CreateList(&l);

//     insertFirst(&l, 0);
//     insertFirst(&l, 1);
//     insertFirst(&l, 2);
//     insertFirst(&l, 3);
//     insertFirst(&l, 0);
//     insertFirst(&l, 2);
//     insertFirst(&l, 3);
//     insertFirst(&l, 0);
    

//     displayList(l);
//     mergeList(&l);
//     displayList(l);
// }