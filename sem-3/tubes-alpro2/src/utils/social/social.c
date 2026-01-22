#include "social.h"

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

void createSocial(Social *s, char *follower_id, char *following_id) {
    my_strncpy(s->follower_id, follower_id, MAX_USER_ID_LEN - 1);
    s->follower_id[MAX_USER_ID_LEN - 1] = '\0';
    
    my_strncpy(s->following_id, following_id, MAX_USER_ID_LEN - 1);
    s->following_id[MAX_USER_ID_LEN - 1] = '\0';
}

char* getFollowerID(Social *s) {
    return s->follower_id;
}

char* getFollowingID(Social *s) {
    return s->following_id;
}

