#ifndef POST_H
#define POST_H

#include "../../adt/boolean.h"

#define MAX_POST_ID_LEN 20
#define MAX_TITLE_LEN 200
#define MAX_CONTENT_LEN 2000
#define MAX_DATETIME_LEN 30

typedef struct {
    char post_id[MAX_POST_ID_LEN];
    char subgroddit_id[MAX_POST_ID_LEN];
    char author_id[MAX_POST_ID_LEN];
    char title[MAX_TITLE_LEN];
    char content[MAX_CONTENT_LEN];
    char created_at[MAX_DATETIME_LEN];
    int upvotes;
    int downvotes;
} Post;


void createPost(Post *p, char *post_id, char *subgroddit_id, char *author_id, 
                char *title, char *content, char *created_at, int upvotes, int downvotes);


char* getPostID(Post *p);
char* getPostSubgrodditID(Post *p);
char* getPostAuthorID(Post *p);
char* getPostTitle(Post *p);
int getPostUpvotes(Post *p);
int getPostDownvotes(Post *p);


void setPostUpvotes(Post *p, int upvotes);
void setPostDownvotes(Post *p, int downvotes);

#endif

