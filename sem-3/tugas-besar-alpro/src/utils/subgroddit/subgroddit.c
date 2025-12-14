#include "subgroddit.h"
#include "../../adt/linkedlist/linkedlist.h"


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

static int my_strlen(const char *s) {
    int len = 0;
    while (s[len] != '\0') {
        len++;
    }
    return len;
}

void createSubgroddit(Subgroddit *s, char *subgroddit_id, char *name) {
    my_strncpy(s->subgroddit_id, subgroddit_id, MAX_SUBGRODDIT_ID_LEN - 1);
    s->subgroddit_id[MAX_SUBGRODDIT_ID_LEN - 1] = '\0';
    
    my_strncpy(s->name, name, MAX_SUBGRODDIT_NAME_LEN - 1);
    s->name[MAX_SUBGRODDIT_NAME_LEN - 1] = '\0';
    
    createList(&s->post_list);
}

char* getSubgrodditID(Subgroddit *s) {
    return s->subgroddit_id;
}

char* getSubgrodditName(Subgroddit *s) {
    return s->name;
}

void setSubgrodditID(Subgroddit *s, char *subgroddit_id) {
    my_strncpy(s->subgroddit_id, subgroddit_id, MAX_SUBGRODDIT_ID_LEN - 1);
    s->subgroddit_id[MAX_SUBGRODDIT_ID_LEN - 1] = '\0';
}

void setSubgrodditName(Subgroddit *s, char *name) {
    my_strncpy(s->name, name, MAX_SUBGRODDIT_NAME_LEN - 1);
    s->name[MAX_SUBGRODDIT_NAME_LEN - 1] = '\0';
}

boolean isValidSubgrodditName(char *name) {
    // Nama diawali dengan "r/"
    if (name == NULL || my_strlen(name) < 3) {
        return false;
    }
    if (name[0] != 'r' || name[1] != '/') {
        return false;
    }
    // Setelah "r/" harus ada minimal 1 karakter
    if (name[2] == '\0') {
        return false;
    }
    return true;
}

void generateSubgrodditID(char *dest, int count) {
    int idx = count + 1;
    dest[0] = 'S';
    dest[1] = '0' + (idx / 100) % 10;
    dest[2] = '0' + (idx / 10) % 10;
    dest[3] = '0' + idx % 10;
    dest[4] = '\0';
}

void addPostToSubgroddit(Subgroddit *s, int post_index) {
    insertLast(&s->post_list, post_index);
}

void removePostFromSubgroddit(Subgroddit *s, int post_index) {
    Address p = s->post_list;
    int list_index = 0;
    
    while (p != NULL) {
        if (p->data == post_index) {
            int dummy;
            deleteAt(&s->post_list, list_index, &dummy);
            return;
        }
        p = p->next;
        list_index++;
    }
}

boolean isPostInSubgroddit(Subgroddit *s, int post_index) {
    Address p = s->post_list;
    while (p != NULL) {
        if (p->data == post_index) {
            return true;
        }
        p = p->next;
    }
    return false;
}

int getPostCount(Subgroddit *s) {
    return length(s->post_list);
}

void clearPostList(Subgroddit *s) {
    clearList(&s->post_list);
}

void updatePostIndices(Subgroddit *s, int deleted_index) {
    Address p = s->post_list;
    while (p != NULL) {
        if (p->data > deleted_index) {
            p->data = p->data - 1;
        }
        p = p->next;
    }
}

