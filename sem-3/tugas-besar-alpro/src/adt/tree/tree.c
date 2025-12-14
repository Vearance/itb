#include "tree.h"
#include <stdio.h>
#include <stdlib.h>


AddressTree newTreeNode(Comment *comment) {
    AddressTree p = (AddressTree)malloc(sizeof(TreeNode));
    if (p != NULL) {
        p->comment = comment;
        p->firstChild = NULL;
        p->nextSibling = NULL;
        p->depth = 0;
    }
    return p;
}

void createTree(Tree *t) {
    *t = NULL;
}


void clearTree(Tree *t) {
    if (*t != NULL) {
        clearTree(&((*t)->firstChild));
        clearTree(&((*t)->nextSibling));
        free(*t);
        *t = NULL;
    }
}


boolean isTreeEmpty(Tree t) {
    return t == NULL;
}


void insertChild(Tree *parent, Comment *childComment) {
    if (*parent == NULL) {
        return;
    }
    
    AddressTree newChild = newTreeNode(childComment);
    if (newChild == NULL) {
        return;
    }
    
    newChild->depth = (*parent)->depth + 1;
    
    
    if ((*parent)->firstChild == NULL) {
        (*parent)->firstChild = newChild;
    } else {
        
        AddressTree current = (*parent)->firstChild;
        while (current->nextSibling != NULL) {
            current = current->nextSibling;
        }
        current->nextSibling = newChild;
    }
}

AddressTree findNodeByCommentID(Tree t, int comment_id) {
    if (t == NULL) {
        return NULL;
    }
    
    if (getCommentID(t->comment) == comment_id) {
        return t;
    }
    
    AddressTree found = findNodeByCommentID(t->firstChild, comment_id);
    if (found != NULL) {
        return found;
    }
    
    return findNodeByCommentID(t->nextSibling, comment_id);
}

void deleteSubtree(Tree *t, int comment_id) {
    if (*t == NULL) {
        return;
    }
    
    if (getCommentID((*t)->comment) == comment_id) {
        clearTree(t);
        return;
    }
    
    if ((*t)->firstChild != NULL) {
        if (getCommentID((*t)->firstChild->comment) == comment_id) {
            AddressTree toDelete = (*t)->firstChild;
            (*t)->firstChild = toDelete->nextSibling;
            clearTree(&toDelete);
            return;
        }
        
        AddressTree current = (*t)->firstChild;
        while (current->nextSibling != NULL) {
            if (getCommentID(current->nextSibling->comment) == comment_id) {
                AddressTree toDelete = current->nextSibling;
                current->nextSibling = toDelete->nextSibling;
                clearTree(&toDelete);
                return;
            }
            current = current->nextSibling;
        }
    }
    
    deleteSubtree(&((*t)->firstChild), comment_id);
    deleteSubtree(&((*t)->nextSibling), comment_id);
}


void printTreePreOrder(Tree t, int indent, void (*printFunc)(Comment*, int)) {
    if (t == NULL) {
        return;
    }
    
    if (printFunc != NULL) {
        printFunc(t->comment, indent);
    } else {
        for (int i = 0; i < indent; i++) {
            printf("  ");
        }
        printf("[%d] %s: %s (↑%d ↓%d)\n",
               getCommentID(t->comment),
               getCommentAuthorID(t->comment),
               getCommentContent(t->comment),
               getCommentUpvotes(t->comment),
               getCommentDownvotes(t->comment));
    }
    
    printTreePreOrder(t->firstChild, indent + 1, printFunc);
    
    printTreePreOrder(t->nextSibling, indent, printFunc);
}

void printTreeInOrder(Tree t, int indent, void (*printFunc)(Comment*, int)) {
    if (t == NULL) {
        return;
    }
    
    printTreeInOrder(t->firstChild, indent + 1, printFunc);
    
    if (printFunc != NULL) {
        printFunc(t->comment, indent);
    } else {
        for (int i = 0; i < indent; i++) {
            printf("  ");
        }
        printf("[%d] %s: %s (↑%d ↓%d)\n",
               getCommentID(t->comment),
               getCommentAuthorID(t->comment),
               getCommentContent(t->comment),
               getCommentUpvotes(t->comment),
               getCommentDownvotes(t->comment));
    }
    
    printTreeInOrder(t->nextSibling, indent, printFunc);
}

void printTreePostOrder(Tree t, int indent, void (*printFunc)(Comment*, int)) {
    if (t == NULL) {
        return;
    }
    
    printTreePostOrder(t->firstChild, indent + 1, printFunc);
    
    printTreePostOrder(t->nextSibling, indent, printFunc);
    
    if (printFunc != NULL) {
        printFunc(t->comment, indent);
    } else {
        for (int i = 0; i < indent; i++) {
            printf("  ");
        }
        printf("[%d] %s: %s (↑%d ↓%d)\n",
               getCommentID(t->comment),
               getCommentAuthorID(t->comment),
               getCommentContent(t->comment),
               getCommentUpvotes(t->comment),
               getCommentDownvotes(t->comment));
    }
}

int getTreeDepth(Tree t) {
    if (t == NULL) {
        return 0;
    }
    
    int maxDepth = t->depth;
    int childDepth = getTreeDepth(t->firstChild);
    int siblingDepth = getTreeDepth(t->nextSibling);
    
    if (childDepth > maxDepth) {
        maxDepth = childDepth;
    }
    if (siblingDepth > maxDepth) {
        maxDepth = siblingDepth;
    }
    
    return maxDepth;
}

int countNodes(Tree t) {
    if (t == NULL) {
        return 0;
    }
    
    return 1 + countNodes(t->firstChild) + countNodes(t->nextSibling);
}

void printTreePreOrderSimple(Tree t, int indent) {
    printTreePreOrder(t, indent, NULL);
}

void printTreeInOrderSimple(Tree t, int indent) {
    printTreeInOrder(t, indent, NULL);
}

void printTreePostOrderSimple(Tree t, int indent) {
    printTreePostOrder(t, indent, NULL);
}
