#ifndef VOTING_H
#define VOTING_H

#include "../../adt/boolean.h"

#define MAX_USER_ID_LEN 20
#define MAX_TARGET_ID_LEN 20
#define MAX_TARGET_TYPE_LEN 10
#define MAX_VOTE_TYPE_LEN 10

typedef enum {
    VOTE_POST,
    VOTE_COMMENT
} TargetType;

typedef enum {
    VOTE_UP,
    VOTE_DOWN
} VoteType;

typedef struct {
    char user_id[MAX_USER_ID_LEN];
    TargetType target_type;
    char target_id[MAX_TARGET_ID_LEN];
    VoteType vote_type;
} Vote;


void createVote(Vote *v, char *user_id, TargetType target_type, char *target_id, VoteType vote_type);


char* getVoteUserID(Vote *v);
TargetType getVoteTargetType(Vote *v);
char* getVoteTargetID(Vote *v);
VoteType getVoteType(Vote *v);

#endif

