#include <stdio.h>
#include "post.h"

void displayMenu() {
    printf("\nMenu:\n");
    printf("1. Buat post\n");
    printf("2. Lihat info post\n");
    printf("3. Set upvotes\n");
    printf("4. Set downvotes\n");
    printf("0. Keluar\n");
    printf("Pilihan: ");
}

int main() {
    Post p;
    int choice;
    int votes;

    do {
        displayMenu();
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                createPost(&p, "P001", "S001", "USER001", 
                           "Belajar Algoritma", 
                           "Bagaimana cara belajar algoritma?",
                           "2024-01-15 09:00:00", 10, 2);
                printf("Post dibuat: Belajar Algoritma\n");
                break;
            case 2:
                printf("ID: %s\n", getPostID(&p));
                printf("Subgroddit: %s\n", getPostSubgrodditID(&p));
                printf("Author: %s\n", getPostAuthorID(&p));
                printf("Judul: %s\n", getPostTitle(&p));
                printf("Upvotes: %d, Downvotes: %d\n", getPostUpvotes(&p), getPostDownvotes(&p));
                break;
            case 3:
                printf("Masukkan upvotes baru: ");
                scanf("%d", &votes);
                setPostUpvotes(&p, votes);
                printf("Upvotes diubah menjadi %d\n", getPostUpvotes(&p));
                break;
            case 4:
                printf("Masukkan downvotes baru: ");
                scanf("%d", &votes);
                setPostDownvotes(&p, votes);
                printf("Downvotes diubah menjadi %d\n", getPostDownvotes(&p));
                break;
            case 0:
                printf("Keluar.\n");
                break;
            default:
                printf("Pilihan tidak valid.\n");
        }
    } while (choice != 0);

    return 0;
}
