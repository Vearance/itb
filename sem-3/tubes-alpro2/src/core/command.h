#ifndef COMMAND_H
#define COMMAND_H

#include "database.h"
#include "../adt/boolean.h"
#include "../adt/mesinkata/mesinkata.h"
#include "../adt/tree/tree.h"


void handleCommand(Database *db);


void cmdRegister(Database *db);
void cmdLogin(Database *db);
void cmdLogout(Database *db);
void cmdCreatePost(Database *db);
void cmdViewPost(Database *db);
void cmdDeletePost(Database *db);
void cmdCreateComment(Database *db); 
void cmdDeleteComment(Database *db); 


void cmdUpvotePost(Database *db);
void cmdDownvotePost(Database *db);
void cmdUndoVotePost(Database *db);
void cmdUpvoteComment(Database *db);
void cmdDownvoteComment(Database *db);
void cmdUndoVoteComment(Database *db);

void cmdFollow(Database *db);
void cmdUnfollow(Database *db);
void cmdFollowers(Database *db);
void cmdFollowing(Database *db);
void cmdViewProfile(Database *db);
void cmdSave(Database *db);
void cmdLoad(Database *db);
void cmdHelp();
void cmdSearch(Database *db);

void cmdCreateSubgroddit(Database *db);
void cmdViewSubgroddit(Database *db);

void cmdFriendRecommendation(Database *db);

#endif

