// #include <stdio.h>
// #include "linkedlist.h"

// void displayMenu() {
//     printf("\nMenu:\n");
//     printf("1. Inisialisasi\n");
//     printf("2. Insert first\n");
//     printf("3. Insert last\n");
//     printf("4. Delete first\n");
//     printf("5. Delete last\n");
//     printf("6. Display list\n");
//     printf("7. Length\n");
//     printf("0. Exit\n");
//     printf("Pilihan: ");
// }

// int main() {
//     List L;
//     int choice;
//     int val;

//     do {
//         displayMenu();
//         scanf("%d", &choice);

//         switch (choice) {
//             case 1:
//                 createList(&L);
//                 printf("List diinisialisasi.\n");
//                 break;
//             case 2:
//                 insertFirst(&L, 10);
//                 insertFirst(&L, 20);
//                 insertFirst(&L, 30);
//                 printf("Insert first: 10, 20, 30\n");
//                 displayList(L);
//                 break;
//             case 3:
//                 insertLast(&L, 100);
//                 insertLast(&L, 200);
//                 printf("Insert last: 100, 200\n");
//                 displayList(L);
//                 break;
//             case 4:
//                 if (!isEmpty(L)) {
//                     deleteFirst(&L, &val);
//                     printf("Deleted: %d\n", val);
//                     displayList(L);
//                 } else {
//                     printf("List kosong.\n");
//                 }
//                 break;
//             case 5:
//                 if (!isEmpty(L)) {
//                     deleteLast(&L, &val);
//                     printf("Deleted: %d\n", val);
//                     displayList(L);
//                 } else {
//                     printf("List kosong.\n");
//                 }
//                 break;
//             case 6:
//                 displayList(L);
//                 break;
//             case 7:
//                 printf("Length: %d\n", length(L));
//                 break;
//             case 0:
//                 printf("Keluar.\n");
//                 break;
//             default:
//                 printf("Pilihan tidak valid.\n");
//         }
//     } while (choice != 0);

//     clearList(&L);
//     return 0;
// }
