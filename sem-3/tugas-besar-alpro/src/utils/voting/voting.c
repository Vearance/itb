#include "voting.h"

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

void createVote(Vote *v, char *user_id, TargetType target_type, char *target_id, VoteType vote_type) {
    my_strncpy(v->user_id, user_id, MAX_USER_ID_LEN - 1);
    v->user_id[MAX_USER_ID_LEN - 1] = '\0';
    
    v->target_type = target_type;
    
    my_strncpy(v->target_id, target_id, MAX_TARGET_ID_LEN - 1);
    v->target_id[MAX_TARGET_ID_LEN - 1] = '\0';
    
    v->vote_type = vote_type;
}

char* getVoteUserID(Vote *v) {
    return v->user_id;
}

TargetType getVoteTargetType(Vote *v) {
    return v->target_type;
}

char* getVoteTargetID(Vote *v) {
    return v->target_id;
}

VoteType getVoteType(Vote *v) {
    return v->vote_type;
}

