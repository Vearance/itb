#include "user.h"
#include <stdlib.h>

static char* my_strncpy(char *dest, const char *src, int n) {
    int i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

void createUser(User *u, char *user_id, char *username, char *password, int karma, char *created_at) {
    my_strncpy(u->user_id, user_id, MAX_USER_ID_LEN - 1);
    u->user_id[MAX_USER_ID_LEN - 1] = '\0';
    
    my_strncpy(u->username, username, MAX_USERNAME_LEN - 1);
    u->username[MAX_USERNAME_LEN - 1] = '\0';
    
    my_strncpy(u->password, password, MAX_PASSWORD_LEN - 1);
    u->password[MAX_PASSWORD_LEN - 1] = '\0';
    
    u->karma = karma;
    
    my_strncpy(u->created_at, created_at, MAX_DATETIME_LEN - 1);
    u->created_at[MAX_DATETIME_LEN - 1] = '\0';
}

char* getUserID(User *u) {
    return u->user_id;
}

char* getUsername(User *u) {
    return u->username;
}

int getKarma(User *u) {
    return u->karma;
}

void setKarma(User *u, int karma) {
    u->karma = karma;
}

