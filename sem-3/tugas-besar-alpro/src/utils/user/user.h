#ifndef USER_H
#define USER_H

#include "../../adt/boolean.h"

#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 50
#define MAX_USER_ID_LEN 20
#define MAX_DATETIME_LEN 30

typedef struct {
    char user_id[MAX_USER_ID_LEN];
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    int karma;
    char created_at[MAX_DATETIME_LEN];
} User;


void createUser(User *u, char *user_id, char *username, char *password, int karma, char *created_at);


char* getUserID(User *u);
char* getUsername(User *u);
int getKarma(User *u);


void setKarma(User *u, int karma);

#endif

