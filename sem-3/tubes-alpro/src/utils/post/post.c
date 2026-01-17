#include "post.h"

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

void createPost(Post *p, char *post_id, char *subgroddit_id, char *author_id, 
                char *title, char *content, char *created_at, int upvotes, int downvotes) {
    my_strncpy(p->post_id, post_id, MAX_POST_ID_LEN - 1);
    p->post_id[MAX_POST_ID_LEN - 1] = '\0';
    
    my_strncpy(p->subgroddit_id, subgroddit_id, MAX_POST_ID_LEN - 1);
    p->subgroddit_id[MAX_POST_ID_LEN - 1] = '\0';
    
    my_strncpy(p->author_id, author_id, MAX_POST_ID_LEN - 1);
    p->author_id[MAX_POST_ID_LEN - 1] = '\0';
    
    my_strncpy(p->title, title, MAX_TITLE_LEN - 1);
    p->title[MAX_TITLE_LEN - 1] = '\0';
    
    my_strncpy(p->content, content, MAX_CONTENT_LEN - 1);
    p->content[MAX_CONTENT_LEN - 1] = '\0';
    
    my_strncpy(p->created_at, created_at, MAX_DATETIME_LEN - 1);
    p->created_at[MAX_DATETIME_LEN - 1] = '\0';
    
    p->upvotes = upvotes;
    p->downvotes = downvotes;
}

char* getPostID(Post *p) {
    return p->post_id;
}

char* getPostSubgrodditID(Post *p) {
    return p->subgroddit_id;
}

char* getPostAuthorID(Post *p) {
    return p->author_id;
}

char* getPostTitle(Post *p) {
    return p->title;
}

int getPostUpvotes(Post *p) {
    return p->upvotes;
}

int getPostDownvotes(Post *p) {
    return p->downvotes;
}

void setPostUpvotes(Post *p, int upvotes) {
    p->upvotes = upvotes;
}

void setPostDownvotes(Post *p, int downvotes) {
    p->downvotes = downvotes;
}

