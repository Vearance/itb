#ifndef COMMENT_H
#define COMMENT_H

#include "../../adt/boolean.h"

#define MAX_COMMENT_ID_LEN 20
#define MAX_CONTENT_LEN 2000

typedef struct {
    int comment_id;
    char post_id[MAX_COMMENT_ID_LEN];
    char author_id[MAX_COMMENT_ID_LEN];
    int parent_comment_id;  
    char content[MAX_CONTENT_LEN];
    int upvotes;
    int downvotes;
} Comment;


void createComment(Comment *c, int comment_id, char *post_id, char *author_id, 
                   int parent_comment_id, char *content, int upvotes, int downvotes);


int getCommentID(Comment *c);
char* getCommentPostID(Comment *c);
char* getCommentAuthorID(Comment *c);
int getCommentParentID(Comment *c);
char* getCommentContent(Comment *c);
int getCommentUpvotes(Comment *c);
int getCommentDownvotes(Comment *c);


void setCommentUpvotes(Comment *c, int upvotes);
void setCommentDownvotes(Comment *c, int downvotes);

#endif

