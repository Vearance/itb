#ifndef SOCIAL_H
#define SOCIAL_H

#include "../../adt/boolean.h"

#define MAX_USER_ID_LEN 20

typedef struct {
    char follower_id[MAX_USER_ID_LEN];
    char following_id[MAX_USER_ID_LEN];
} Social;


void createSocial(Social *s, char *follower_id, char *following_id);


char* getFollowerID(Social *s);
char* getFollowingID(Social *s);

#endif

