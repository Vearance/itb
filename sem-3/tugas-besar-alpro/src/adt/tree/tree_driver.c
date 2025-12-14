// #include <stdio.h>
// #include <stdlib.h>
// #include "tree.h"

// void displayMenu() {
//     printf("\nMenu:\n");
//     printf("1. Inisialisasi\n");
//     printf("2. Buat root\n");
//     printf("3. Tambah child\n");
//     printf("4. Cari comment\n");
//     printf("5. Print tree\n");
//     printf("6. Count nodes\n");
//     printf("0. Exit\n");
//     printf("Pilihan: ");
// }

// int main() {
//     Tree t;
//     int choice;

//     Comment *c1, *c2, *c3, *c4;

//     do {
//         displayMenu();
//         scanf("%d", &choice);

//         switch (choice) {
//             case 1:
//                 createTree(&t);
//                 printf("Tree diinisialisasi.\n");
//                 break;
//             case 2:
//                 c1 = (Comment*)malloc(sizeof(Comment));
//                 createComment(c1, 1, "post1", "alice", -1, "Ini root comment", 5, 1);
//                 t = newTreeNode(c1);
//                 printf("Root dibuat (ID: 1).\n");
//                 break;
//             case 3:
//                 c2 = (Comment*)malloc(sizeof(Comment));
//                 c3 = (Comment*)malloc(sizeof(Comment));
//                 c4 = (Comment*)malloc(sizeof(Comment));
//                 createComment(c2, 2, "post1", "bob", 1, "Reply pertama", 3, 0);
//                 createComment(c3, 3, "post1", "charlie", 1, "Reply kedua", 2, 1);
//                 createComment(c4, 4, "post1", "diana", 2, "Reply ke reply", 1, 0);
                
//                 AddressTree node = findNodeByCommentID(t, 1);
//                 if (node != NULL) insertChild(&node, c2);
//                 node = findNodeByCommentID(t, 1);
//                 if (node != NULL) insertChild(&node, c3);
//                 node = findNodeByCommentID(t, 2);
//                 if (node != NULL) insertChild(&node, c4);
//                 printf("3 child ditambahkan.\n");
//                 break;
//             case 4:
//                 node = findNodeByCommentID(t, 2);
//                 if (node != NULL) {
//                     printf("Found: ID=%d, Author=%s\n", 
//                            getCommentID(node->comment),
//                            getCommentAuthorID(node->comment));
//                 } else {
//                     printf("Tidak ditemukan.\n");
//                 }
//                 break;
//             case 5:
//                 if (!isTreeEmpty(t)) {
//                     printTreePreOrderSimple(t, 0);
//                 } else {
//                     printf("Tree kosong.\n");
//                 }
//                 break;
//             case 6:
//                 printf("Jumlah node: %d\n", countNodes(t));
//                 break;
//             case 0:
//                 printf("Keluar.\n");
//                 break;
//             default:
//                 printf("Pilihan tidak valid.\n");
//         }
//     } while (choice != 0);

//     clearTree(&t);
//     return 0;
// }
