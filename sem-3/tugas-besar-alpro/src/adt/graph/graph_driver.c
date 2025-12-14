// #include <stdio.h>
// #include "graph.h"

// void displayMenu() {
//     printf("\nMenu:\n");
//     printf("1. Inisialisasi\n");
//     printf("2. Add vertex\n");
//     printf("3. Add edge\n");
//     printf("4. Remove edge\n");
//     printf("5. Cek ada edge\n");
//     printf("6. Info graph\n");
//     printf("0. Exit\n");
//     printf("Pilihan: ");
// }

// int main() {
//     Graph g;
//     int choice;

//     User u1, u2, u3, u4;
//     createUser(&u1, "u1", "alice", "pass", 100, "2025-01-01");
//     createUser(&u2, "u2", "bob", "pass", 80, "2025-01-01");
//     createUser(&u3, "u3", "usermoo", "pass", 120, "2025-01-01");
//     createUser(&u4, "u4", "kebin", "pass", 90, "2025-01-01");

//     do {
//         displayMenu();
//         scanf("%d", &choice);

//         switch (choice) {
//             case 1:
//                 createGraph(&g);
//                 printf("Graph diinisialisasi.\n");
//                 break;
//             case 2:
//                 addVertex(&g, &u1);
//                 addVertex(&g, &u2);
//                 addVertex(&g, &u3);
//                 addVertex(&g, &u4);
//                 printf("4 user ditambahkan: alice, bob, charlie, diana\n");
//                 break;
//             case 3:
//                 addEdge(&g, "u1", "u2");
//                 addEdge(&g, "u1", "u3");
//                 addEdge(&g, "u2", "u4");
//                 printf("Edge ditambahkan: u1->u2, u1->u3, u2->u4\n");
//                 break;
//             case 4:
//                 removeEdge(&g, "u1", "u2");
//                 printf("Edge u1->u2 dihapus.\n");
//                 break;
//             case 5:
//                 printf("u1->u2: %s\n", hasEdge(&g, "u1", "u2") ? "ada" : "tidak ada");
//                 printf("u1->u3: %s\n", hasEdge(&g, "u1", "u3") ? "ada" : "tidak ada");
//                 break;
//             case 6:
//                 printf("Jumlah vertex: %d\n", g.vertexCount);
//                 for (int i = 0; i < g.vertexCount; i++) {
//                     if (g.vertices[i] != NULL) {
//                         printf("  - %s\n", getUserID(g.vertices[i]));
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
