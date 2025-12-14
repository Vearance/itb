// #include <stdio.h>
// #include "mesinkar.h"

// void displayMenu() {
//     printf("\nMenu:\n");
//     printf("1. Baca dari stdin (akhiri dengan ';')\n");
//     printf("2. Baca dari file\n");
//     printf("3. Test IgnoreBlank\n");
//     printf("0. Exit\n");
//     printf("Pilihan: ");
// }

// int main() {
//     int choice;
//     char filename[256];
//     FILE *file;

//     printf("=== Driver Mesin Karakter ===\n");

//     do {
//         displayMenu();
//         scanf("%d", &choice);
//         getchar();

//         switch (choice) {
//             case 1:
//                 printf("Masukkan teks: ");
//                 START();
//                 while (!EOP) {
//                     printf("CC = '%c'\n", CC);
//                     ADV();
//                 }
//                 printf("EOP tercapai.\n");
//                 break;
//             case 2:
//                 printf("Nama file: ");
//                 scanf("%s", filename);
//                 file = fopen(filename, "r");
//                 if (file == NULL) {
//                     printf("Gagal membuka file.\n");
//                 } else {
//                     STARTFILE(file);
//                     while (!EOP) {
//                         printf("CC = '%c'\n", CC);
//                         ADV();
//                     }
//                     printf("EOP tercapai.\n");
//                 }
//                 break;
//             case 3:
//                 printf("Masukkan teks dengan spasi: ");
//                 START();
//                 while (!EOP) {
//                     IgnoreBlank();
//                     if (!EOP) {
//                         printf("CC = '%c'\n", CC);
//                         ADV();
//                     }
//                 }
//                 break;
//             case 0:
//                 printf("Keluar.\n");
//                 break;
//             default:
//                 printf("Pilihan tidak valid.\n");
//         }
//     } while (choice != 0);

//     return 0;
// }
