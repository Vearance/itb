#include <stdio.h>
#include "user.h"

void displayMenu() {
    printf("\nMenu:\n");
    printf("1. Buat user\n");
    printf("2. Lihat info user\n");
    printf("3. Set karma\n");
    printf("0. Keluar\n");
    printf("Pilihan: ");
}

int main() {
    User u;
    int choice;
    int karma;

    do {
        displayMenu();
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                createUser(&u, "USER001", "alice", "pass123", 100, "2024-01-01 10:00:00");
                printf("User dibuat: alice\n");
                break;
            case 2:
                printf("ID: %s\n", getUserID(&u));
                printf("Username: %s\n", getUsername(&u));
                printf("Karma: %d\n", getKarma(&u));
                break;
            case 3:
                printf("Masukkan karma baru: ");
                scanf("%d", &karma);
                setKarma(&u, karma);
                printf("Karma diubah menjadi %d\n", getKarma(&u));
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
