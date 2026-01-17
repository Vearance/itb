#include <stdio.h>
#include "voting.h"

void displayMenu() {
    printf("\nMenu:\n");
    printf("1. Buat vote upvote post\n");
    printf("2. Buat vote downvote comment\n");
    printf("3. Lihat info vote\n");
    printf("0. Keluar\n");
    printf("Pilihan: ");
}

int main() {
    Vote v;
    int choice;

    do {
        displayMenu();
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                createVote(&v, "USER001", VOTE_POST, "P001", VOTE_UP);
                printf("Vote dibuat: upvote untuk post P001\n");
                break;
            case 2:
                createVote(&v, "USER002", VOTE_COMMENT, "5", VOTE_DOWN);
                printf("Vote dibuat: downvote untuk comment 5\n");
                break;
            case 3:
                printf("User: %s\n", getVoteUserID(&v));
                printf("Target: %s (%s)\n", getVoteTargetID(&v), 
                       getVoteTargetType(&v) == VOTE_POST ? "POST" : "COMMENT");
                printf("Tipe: %s\n", getVoteType(&v) == VOTE_UP ? "UPVOTE" : "DOWNVOTE");
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
