// #include <stdio.h>
// #include <string.h>
// #include "mesinkata.h"

// void displayMenu() {
//     printf("\nMenu:\n");
//     printf("1. Baca kata dari stdin\n");
//     printf("2. Baca kata dari file\n");
//     printf("3. Test StringToKata\n");
//     printf("4. Test IsKataEqual\n");
//     printf("5. Test KataToInt\n");
//     printf("6. Test IsKataInt\n");
//     printf("0. Exit\n");
//     printf("Pilihan: ");
// }

// int main() {
//     int choice;
//     char filename[256];
//     char str1[NMax + 1], str2[NMax + 1];
//     Kata kata1, kata2;
//     FILE *file;
//     int wordCount;

//     printf("=== Driver Mesin Kata ===\n");

//     do {
//         displayMenu();
//         scanf("%d", &choice);
//         getchar();

//         switch (choice) {
//             case 1:
//                 printf("Masukkan teks (akhiri dengan ';'): ");
//                 STARTKATA();
//                 wordCount = 1;
//                 while (!EndKata) {
//                     printf("Kata %d: \"%s\" (len: %d)\n", wordCount, KataToString(CKata), KataLength(CKata));
//                     wordCount++;
//                     ADVKATA();
//                 }
//                 break;
//             case 2:
//                 printf("Nama file: ");
//                 scanf("%s", filename);
//                 file = fopen(filename, "r");
//                 if (file == NULL) {
//                     printf("Gagal membuka file.\n");
//                 } else {
//                     STARTKATAFILE(file);
//                     wordCount = 1;
//                     while (!EndKata) {
//                         printf("Kata %d: \"%s\"\n", wordCount, KataToString(CKata));
//                         wordCount++;
//                         ADVKATA();
//                     }
//                 }
//                 break;
//             case 3:
//                 printf("String: ");
//                 fgets(str1, NMax, stdin);
//                 str1[strcspn(str1, "\n")] = '\0';
//                 kata1 = StringToKata(str1);
//                 printf("Hasil: \"%s\" (len: %d)\n", KataToString(kata1), KataLength(kata1));
//                 break;
//             case 4:
//                 printf("String 1: ");
//                 fgets(str1, NMax, stdin);
//                 str1[strcspn(str1, "\n")] = '\0';
//                 printf("String 2: ");
//                 fgets(str2, NMax, stdin);
//                 str2[strcspn(str2, "\n")] = '\0';
//                 kata1 = StringToKata(str1);
//                 kata2 = StringToKata(str2);
//                 printf("Hasil: %s\n", IsKataEqual(kata1, kata2) ? "SAMA" : "TIDAK SAMA");
//                 break;
//             case 5:
//                 printf("String angka: ");
//                 fgets(str1, NMax, stdin);
//                 str1[strcspn(str1, "\n")] = '\0';
//                 kata1 = StringToKata(str1);
//                 printf("Hasil: %d\n", KataToInt(kata1));
//                 break;
//             case 6:
//                 printf("String: ");
//                 fgets(str1, NMax, stdin);
//                 str1[strcspn(str1, "\n")] = '\0';
//                 kata1 = StringToKata(str1);
//                 printf("Hasil: %s\n", IsKataInt(kata1) ? "YA, integer" : "BUKAN integer");
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
