#include "comment.h"

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

void createComment(Comment *c, int comment_id, char *post_id, char *author_id, 
                   int parent_comment_id, char *content, int upvotes, int downvotes) {
    c->comment_id = comment_id;
    
    my_strncpy(c->post_id, post_id, MAX_COMMENT_ID_LEN - 1);
    c->post_id[MAX_COMMENT_ID_LEN - 1] = '\0';
    
    my_strncpy(c->author_id, author_id, MAX_COMMENT_ID_LEN - 1);
    c->author_id[MAX_COMMENT_ID_LEN - 1] = '\0';
    
    c->parent_comment_id = parent_comment_id;
    
    my_strncpy(c->content, content, MAX_CONTENT_LEN - 1);
    c->content[MAX_CONTENT_LEN - 1] = '\0';
    
    c->upvotes = upvotes;
    c->downvotes = downvotes;
}

int getCommentID(Comment *c) {
    return c->comment_id;
}

char* getCommentPostID(Comment *c) {
    return c->post_id;
}

char* getCommentAuthorID(Comment *c) {
    return c->author_id;
}

int getCommentParentID(Comment *c) {
    return c->parent_comment_id;
}

char* getCommentContent(Comment *c) {
    return c->content;
}

int getCommentUpvotes(Comment *c) {
    return c->upvotes;
}

int getCommentDownvotes(Comment *c) {
    return c->downvotes;
}

void setCommentUpvotes(Comment *c, int upvotes) {
    c->upvotes = upvotes;
}

void setCommentDownvotes(Comment *c, int downvotes) {
    c->downvotes = downvotes;
}

