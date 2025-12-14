#ifndef TREE_H
#define TREE_H

#include "../boolean.h"
#include "../../utils/comment/comment.h"

typedef struct TreeNode *AddressTree;
typedef struct TreeNode {
    Comment *comment;        
    AddressTree firstChild;  
    AddressTree nextSibling; 
    int depth;               
} TreeNode;

typedef AddressTree Tree;


AddressTree newTreeNode(Comment *comment);
void createTree(Tree *t);


void clearTree(Tree *t);


boolean isTreeEmpty(Tree t);


void insertChild(Tree *parent, Comment *childComment);
AddressTree findNodeByCommentID(Tree t, int comment_id);
void deleteSubtree(Tree *t, int comment_id);


void printTreePreOrder(Tree t, int indent, void (*printFunc)(Comment*, int));
void printTreeInOrder(Tree t, int indent, void (*printFunc)(Comment*, int));
void printTreePostOrder(Tree t, int indent, void (*printFunc)(Comment*, int));
void printTreePreOrderSimple(Tree t, int indent);
void printTreeInOrderSimple(Tree t, int indent);
void printTreePostOrderSimple(Tree t, int indent);

int getTreeDepth(Tree t);
int countNodes(Tree t);

#endif
