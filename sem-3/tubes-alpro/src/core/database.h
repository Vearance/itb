#ifndef DATABASE_H
#define DATABASE_H

#include "../utils/user/user.h"
#include "../utils/post/post.h"
#include "../utils/comment/comment.h"
#include "../utils/subgroddit/subgroddit.h"
#include "../utils/voting/voting.h"
#include "../utils/social/social.h"
#include "../adt/linkedlist/linkedlist.h"
#include "../adt/graph/graph.h"
#include "../adt/tree/tree.h"
#include "../adt/boolean.h"

#define MAX_USERS 1000
#define MAX_POSTS 1000
#define MAX_COMMENTS 10000
#define MAX_SUBGRODDITS 100
#define MAX_VOTES 10000
#define MAX_SOCIALS 10000
#define MAX_BLACKLIST 100
#define MAX_BLACKLIST_WORD_LEN 50

typedef struct {
    User users[MAX_USERS];
    int user_count;
    
    Post posts[MAX_POSTS];
    int post_count;
    
    Comment comments[MAX_COMMENTS];
    int comment_count;
    
    Subgroddit subgroddits[MAX_SUBGRODDITS];
    int subgroddit_count;
    
    Vote votes[MAX_VOTES];
    int vote_count;
    
    Social socials[MAX_SOCIALS];
    int social_count;
    
    Graph social_graph;  

    char blacklist[MAX_BLACKLIST][MAX_BLACKLIST_WORD_LEN];
    int blacklist_count;

    char current_user_id[MAX_USER_ID_LEN];
    char last_loaded_folder[256];
} Database;


void createDatabase(Database *db);


boolean loadUsersFromCSV(Database *db, char *filename);
boolean loadPostsFromCSV(Database *db, char *filename); 
boolean loadCommentsFromCSV(Database *db, char *filename); 
boolean loadSubgrodditsFromCSV(Database *db, char *filename); 
boolean loadVotesFromCSV(Database *db, char *filename); 
boolean loadSocialsFromCSV(Database *db, char *filename); 


boolean saveUsersToCSV(Database *db, char *filename);
boolean savePostsToCSV(Database *db, char *filename); 
boolean saveCommentsToCSV(Database *db, char *filename); 
boolean saveSubgrodditsToCSV(Database *db, char *filename); 
boolean saveVotesToCSV(Database *db, char *filename); 
boolean saveSocialsToCSV(Database *db, char *filename); 


User* findUserByID(Database *db, char *user_id);
User* findUserByUsername(Database *db, char *username);
Post* findPostByID(Database *db, char *post_id);
Comment* findCommentByID(Database *db, int comment_id); 
Subgroddit* findSubgrodditByID(Database *db, char *subgroddit_id);
Subgroddit* findSubgrodditByName(Database *db, char *name);


boolean addSubgroddit(Database *db, char *name);
int getPostCountBySubgroddit(Database *db, char *subgroddit_id);
Post** getPostsBySubgroddit(Database *db, char *subgroddit_id, int *count);
void sortPostsByHot(Post **posts, int count, boolean descending);
void sortPostsByNew(Post **posts, int count, boolean descending);


boolean registerUser(Database *db, char *username, char *password);
void generateUserID(char *dest, int count);
boolean isValidPassword(char *password);


boolean login(Database *db, char *username, char *password);
void logout(Database *db);
boolean isLoggedIn(Database *db);
char* getCurrentUserID(Database *db);


Vote* findVote(Database *db, char *user_id, TargetType target_type, char *target_id);
boolean addVote(Database *db, char *user_id, TargetType target_type, char *target_id, VoteType vote_type);
boolean removeVote(Database *db, char *user_id, TargetType target_type, char *target_id);
boolean updateVote(Database *db, char *user_id, TargetType target_type, char *target_id, VoteType new_vote_type);


Tree buildCommentTree(Database *db, char *post_id);

boolean loadBlacklistFromCSV(Database *db, char *filename);
char* containsBlacklistedWord(Database *db, const char *text);

#endif

