#ifndef SUBGRODDIT_H
#define SUBGRODDIT_H

#include <stdio.h>

#include "../../adt/boolean.h"
#include "../../adt/linkedlist/linkedlist.h"

#define MAX_SUBGRODDIT_ID_LEN 20
#define MAX_SUBGRODDIT_NAME_LEN 100

typedef struct {
    char subgroddit_id[MAX_SUBGRODDIT_ID_LEN];
    char name[MAX_SUBGRODDIT_NAME_LEN];
    List post_list;  
} Subgroddit;


void createSubgroddit(Subgroddit *s, char *subgroddit_id, char *name);


char* getSubgrodditID(Subgroddit *s);
char* getSubgrodditName(Subgroddit *s);


void setSubgrodditID(Subgroddit *s, char *subgroddit_id);
void setSubgrodditName(Subgroddit *s, char *name);


void addPostToSubgroddit(Subgroddit *s, int post_index);
void removePostFromSubgroddit(Subgroddit *s, int post_index);
boolean isPostInSubgroddit(Subgroddit *s, int post_index);
int getPostCount(Subgroddit *s);
void clearPostList(Subgroddit *s);
void updatePostIndices(Subgroddit *s, int deleted_index);


boolean isValidSubgrodditName(char *name);
void generateSubgrodditID(char *dest, int count);

#endif

