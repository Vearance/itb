#include "database.h"
#include "../adt/linkedlist/linkedlist.h"
#include "../adt/tree/tree.h"
#include "../adt/mesinkata/mesinkata.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static int my_strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

static int my_strlen(const char *s) {
    int len = 0;
    while (s[len] != '\0') {
        len++;
    }
    return len;
}

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

// static int my_strcspn(const char *s, const char *reject) {
//     int count = 0;
//     while (s[count] != '\0') {
//         int found = 0;
//         for (int i = 0; reject[i] != '\0'; i++) {
//             if (s[count] == reject[i]) {
//                 found = 1;
//                 break;
//             }
//         }
//         if (found) break;
//         count++;
//     }
//     return count;
// }

void createDatabase(Database *db) {
    db->user_count = 0;
    db->post_count = 0;
    db->comment_count = 0;
    db->subgroddit_count = 0;
    db->vote_count = 0;
    db->social_count = 0;
    db->blacklist_count = 0;
    db->current_user_id[0] = '\0';
    db->last_loaded_folder[0] = '\0';
    createGraph(&db->social_graph);
}



boolean loadUsersFromCSV(Database *db, char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return false;
    }
    
    STARTKATACSV(file);
    
    /* Skip header line */
    SkipLine();
    
    db->user_count = 0;
    while (!EndKata && db->user_count < MAX_USERS) {
        char user_id[50], username[256], password[50], created_at[50];
        int karma;
        
        /* Read user_id */
        ADVKATACSV();
        if (EndKata || EndLine()) break;
        my_strncpy(user_id, CKata.TabKata, sizeof(user_id) - 1);
        user_id[sizeof(user_id) - 1] = '\0';
        
        /* Read username */
        ADVKATACSV();
        if (EndLine()) continue;
        my_strncpy(username, CKata.TabKata, sizeof(username) - 1);
        username[sizeof(username) - 1] = '\0';
        
        /* Read password */
        ADVKATACSV();
        if (EndLine()) continue;
        my_strncpy(password, CKata.TabKata, sizeof(password) - 1);
        password[sizeof(password) - 1] = '\0';
        
        /* Read karma */
        ADVKATACSV();
        if (EndLine()) continue;
        karma = KataToInt(CKata);
        
        /* Read created_at */
        ADVKATACSV();
        my_strncpy(created_at, CKata.TabKata, sizeof(created_at) - 1);
        created_at[sizeof(created_at) - 1] = '\0';
        
        if (user_id[0] != '\0') {
            createUser(&db->users[db->user_count], user_id, username, password, karma, created_at);
            addVertex(&db->social_graph, &db->users[db->user_count]);
            db->user_count++;
        }
        
        /* Skip to next line if not already at end of line */
        if (!EndLine() && !EndKata) {
            SkipLine();
        }
    }
    
    fclose(file);
    return true;
}

boolean loadPostsFromCSV(Database *db, char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return false;
    }
    
    STARTKATACSV(file);
    
    /* Skip header line */
    SkipLine();
    
    db->post_count = 0;
    while (!EndKata && db->post_count < MAX_POSTS) {
        char post_id[50], subgroddit_id[50], author_id[50], title[300], content[2048], created_at[50];
        int upvotes, downvotes;
        
        /* Read post_id */
        ADVKATACSV();
        if (EndKata || EndLine()) break;
        my_strncpy(post_id, CKata.TabKata, sizeof(post_id) - 1);
        post_id[sizeof(post_id) - 1] = '\0';
        
        /* Read subgroddit_id */
        ADVKATACSV();
        if (EndLine()) continue;
        my_strncpy(subgroddit_id, CKata.TabKata, sizeof(subgroddit_id) - 1);
        subgroddit_id[sizeof(subgroddit_id) - 1] = '\0';
        
        /* Read author_id */
        ADVKATACSV();
        if (EndLine()) continue;
        my_strncpy(author_id, CKata.TabKata, sizeof(author_id) - 1);
        author_id[sizeof(author_id) - 1] = '\0';
        
        /* Read title */
        ADVKATACSV();
        if (EndLine()) continue;
        my_strncpy(title, CKata.TabKata, sizeof(title) - 1);
        title[sizeof(title) - 1] = '\0';
        
        /* Read content */
        ADVKATACSV();
        if (EndLine()) continue;
        my_strncpy(content, CKata.TabKata, sizeof(content) - 1);
        content[sizeof(content) - 1] = '\0';
        
        /* Read created_at */
        ADVKATACSV();
        if (EndLine()) continue;
        my_strncpy(created_at, CKata.TabKata, sizeof(created_at) - 1);
        created_at[sizeof(created_at) - 1] = '\0';
        
        /* Read upvotes */
        ADVKATACSV();
        if (EndLine()) continue;
        upvotes = KataToInt(CKata);
        
        /* Read downvotes */
        ADVKATACSV();
        downvotes = KataToInt(CKata);
        
        if (post_id[0] != '\0') {
            createPost(&db->posts[db->post_count], post_id, subgroddit_id, author_id, 
                      title, content, created_at, upvotes, downvotes);
            
            Subgroddit *subgroddit = findSubgrodditByID(db, subgroddit_id);
            if (subgroddit != NULL) {
                addPostToSubgroddit(subgroddit, db->post_count);
            }
            
            db->post_count++;
        }
        
        /* Skip to next line if not already at end of line */
        if (!EndLine() && !EndKata) {
            SkipLine();
        }
    }
    
    fclose(file);
    return true;
}

boolean loadCommentsFromCSV(Database *db, char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return false;
    }
    
    STARTKATACSV(file);
    
    /* Skip header line */
    SkipLine();
    
    db->comment_count = 0;
    while (!EndKata && db->comment_count < MAX_COMMENTS) {
        int comment_id, parent_comment_id, upvotes, downvotes;
        char post_id[50], author_id[50], content[2048];
        
        /* Read comment_id */
        ADVKATACSV();
        if (EndKata || EndLine()) break;
        comment_id = KataToInt(CKata);
        
        /* Read post_id */
        ADVKATACSV();
        if (EndLine()) continue;
        my_strncpy(post_id, CKata.TabKata, sizeof(post_id) - 1);
        post_id[sizeof(post_id) - 1] = '\0';
        
        /* Read author_id */
        ADVKATACSV();
        if (EndLine()) continue;
        my_strncpy(author_id, CKata.TabKata, sizeof(author_id) - 1);
        author_id[sizeof(author_id) - 1] = '\0';
        
        /* Read parent_comment_id */
        ADVKATACSV();
        if (EndLine()) continue;
        parent_comment_id = KataToInt(CKata);
        
        /* Read content */
        ADVKATACSV();
        if (EndLine()) continue;
        my_strncpy(content, CKata.TabKata, sizeof(content) - 1);
        content[sizeof(content) - 1] = '\0';
        
        /* Read upvotes */
        ADVKATACSV();
        if (EndLine()) continue;
        upvotes = KataToInt(CKata);
        
        /* Read downvotes */
        ADVKATACSV();
        downvotes = KataToInt(CKata);
        
        if (post_id[0] != '\0') {
            createComment(&db->comments[db->comment_count], comment_id, post_id, author_id, 
                         parent_comment_id, content, upvotes, downvotes);
            db->comment_count++;
        }
        
        /* Skip to next line if not already at end of line */
        if (!EndLine() && !EndKata) {
            SkipLine();
        }
    }
    
    fclose(file);
    return true;
}

boolean loadSubgrodditsFromCSV(Database *db, char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return false;
    }
    
    STARTKATACSV(file);
    
    /* Skip header line */
    SkipLine();
    
    db->subgroddit_count = 0;
    while (!EndKata && db->subgroddit_count < MAX_SUBGRODDITS) {
        char subgroddit_id[50], name[256];
        
        /* Read subgroddit_id */
        ADVKATACSV();
        if (EndKata || EndLine()) break;
        my_strncpy(subgroddit_id, CKata.TabKata, sizeof(subgroddit_id) - 1);
        subgroddit_id[sizeof(subgroddit_id) - 1] = '\0';
        
        /* Read name */
        ADVKATACSV();
        my_strncpy(name, CKata.TabKata, sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
        
        if (subgroddit_id[0] != '\0') {
            createSubgroddit(&db->subgroddits[db->subgroddit_count], subgroddit_id, name);
            db->subgroddit_count++;
        }
        
        /* Skip to next line if not already at end of line */
        if (!EndLine() && !EndKata) {
            SkipLine();
        }
    }
    
    fclose(file);
    return true;
}

boolean loadVotesFromCSV(Database *db, char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return false;
    }
    
    STARTKATACSV(file);
    
    /* Skip header line */
    SkipLine();
    
    db->vote_count = 0;
    while (!EndKata && db->vote_count < MAX_VOTES) {
        char user_id[50], target_type[20], target_id[50], vote_type[20];
        
        /* Read user_id */
        ADVKATACSV();
        if (EndKata || EndLine()) break;
        my_strncpy(user_id, CKata.TabKata, sizeof(user_id) - 1);
        user_id[sizeof(user_id) - 1] = '\0';
        
        /* Read target_type */
        ADVKATACSV();
        if (EndLine()) continue;
        my_strncpy(target_type, CKata.TabKata, sizeof(target_type) - 1);
        target_type[sizeof(target_type) - 1] = '\0';
        
        /* Read target_id */
        ADVKATACSV();
        if (EndLine()) continue;
        my_strncpy(target_id, CKata.TabKata, sizeof(target_id) - 1);
        target_id[sizeof(target_id) - 1] = '\0';
        
        /* Read vote_type */
        ADVKATACSV();
        my_strncpy(vote_type, CKata.TabKata, sizeof(vote_type) - 1);
        vote_type[sizeof(vote_type) - 1] = '\0';
        
        if (user_id[0] != '\0') {
            TargetType tt = (my_strcmp(target_type, "POST") == 0) ? VOTE_POST : VOTE_COMMENT;
            VoteType vt = (my_strcmp(vote_type, "UPVOTE") == 0) ? VOTE_UP : VOTE_DOWN;
            
            createVote(&db->votes[db->vote_count], user_id, tt, target_id, vt);
            db->vote_count++;
        }
        
        /* Skip to next line if not already at end of line */
        if (!EndLine() && !EndKata) {
            SkipLine();
        }
    }
    
    fclose(file);
    return true;
}

boolean loadSocialsFromCSV(Database *db, char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return false;
    }
    
    STARTKATACSV(file);
    
    /* Skip header line */
    SkipLine();
    
    db->social_count = 0;
    while (!EndKata && db->social_count < MAX_SOCIALS) {
        char follower_id[50], following_id[50];
        
        /* Read follower_id */
        ADVKATACSV();
        if (EndKata || EndLine()) break;
        my_strncpy(follower_id, CKata.TabKata, sizeof(follower_id) - 1);
        follower_id[sizeof(follower_id) - 1] = '\0';
        
        /* Read following_id */
        ADVKATACSV();
        my_strncpy(following_id, CKata.TabKata, sizeof(following_id) - 1);
        following_id[sizeof(following_id) - 1] = '\0';
        
        if (follower_id[0] != '\0' && following_id[0] != '\0') {
            createSocial(&db->socials[db->social_count], follower_id, following_id);
            addEdge(&db->social_graph, follower_id, following_id);
            db->social_count++;
        }
        
        /* Skip to next line if not already at end of line */
        if (!EndLine() && !EndKata) {
            SkipLine();
        }
    }
    
    fclose(file);
    return true;
}


boolean saveUsersToCSV(Database *db, char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file %s for writing\n", filename);
        return false;
    }
    
    fprintf(file, "user_id,username,password,karma,created_at\n");
    for (int i = 0; i < db->user_count; i++) {
        fprintf(file, "%s,%s,%s,%d,%s\n", 
                db->users[i].user_id, db->users[i].username, 
                db->users[i].password, db->users[i].karma, db->users[i].created_at);
    }
    
    fclose(file);
    return true;
}


boolean savePostsToCSV(Database *db, char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file %s for writing\n", filename);
        return false;
    }
    
    fprintf(file, "post_id,subgroddit_id,author_id,title,content,created_at,upvotes,downvotes\n");
    for (int i = 0; i < db->post_count; i++) {
        fprintf(file, "%s,%s,%s,\"%s\",\"%s\",%s,%d,%d\n", 
                db->posts[i].post_id, db->posts[i].subgroddit_id, 
                db->posts[i].author_id, db->posts[i].title, db->posts[i].content,
                db->posts[i].created_at, db->posts[i].upvotes, db->posts[i].downvotes);
    }
    
    fclose(file);
    return true;
}

boolean saveCommentsToCSV(Database *db, char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file %s for writing\n", filename);
        return false;
    }
    
    fprintf(file, "comment_id,post_id,author_id,parent_comment_id,content,upvotes,downvotes\n");
    for (int i = 0; i < db->comment_count; i++) {
        fprintf(file, "%d,%s,%s,%d,\"%s\",%d,%d\n", 
                db->comments[i].comment_id, db->comments[i].post_id, 
                db->comments[i].author_id, db->comments[i].parent_comment_id,
                db->comments[i].content, db->comments[i].upvotes, db->comments[i].downvotes);
    }
    
    fclose(file);
    return true;
}

boolean saveSubgrodditsToCSV(Database *db, char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file %s for writing\n", filename);
        return false;
    }
    
    fprintf(file, "subgroddit_id,name\n");
    for (int i = 0; i < db->subgroddit_count; i++) {
        fprintf(file, "%s,%s\n", 
                db->subgroddits[i].subgroddit_id, db->subgroddits[i].name);
    }
    
    fclose(file);
    return true;
}

boolean saveVotesToCSV(Database *db, char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file %s for writing\n", filename);
        return false;
    }
    
    fprintf(file, "user_id,target_type,target_id,vote_type\n");
    for (int i = 0; i < db->vote_count; i++) {
        const char *target_type = (db->votes[i].target_type == VOTE_POST) ? "POST" : "COMMENT";
        const char *vote_type = (db->votes[i].vote_type == VOTE_UP) ? "UPVOTE" : "DOWNVOTE";
        fprintf(file, "%s,%s,%s,%s\n", 
                db->votes[i].user_id, target_type, db->votes[i].target_id, vote_type);
    }
    
    fclose(file);
    return true;
}

boolean saveSocialsToCSV(Database *db, char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file %s for writing\n", filename);
        return false;
    }
    
    fprintf(file, "follower_id,following_id\n");
    for (int i = 0; i < db->social_count; i++) {
        fprintf(file, "%s,%s\n", 
                db->socials[i].follower_id, db->socials[i].following_id);
    }
    
    fclose(file);
    return true;
}



User* findUserByID(Database *db, char *user_id) {
    for (int i = 0; i < db->user_count; i++) {
        if (my_strcmp(db->users[i].user_id, user_id) == 0) {
            return &db->users[i];
        }
    }
    return NULL;
}

User* findUserByUsername(Database *db, char *username) {
    for (int i = 0; i < db->user_count; i++) {
        if (my_strcmp(db->users[i].username, username) == 0) {
            return &db->users[i];
        }
    }
    return NULL;
}


void generateUserID(char *dest, int count) {
    
    int idx = count + 1;
    dest[0] = 'U';
    dest[1] = 'S';
    dest[2] = 'E';
    dest[3] = 'R';
    dest[4] = '0' + (idx / 100) % 10;
    dest[5] = '0' + (idx / 10) % 10;
    dest[6] = '0' + idx % 10;
    dest[7] = '\0';
}

static void getCurrentDatetime(char *dest, int max_len) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    snprintf(dest, max_len, "%04d-%02d-%02d %02d:%02d:%02d",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);
}

boolean isValidPassword(char *password) {
    int len = my_strlen(password);
    
    return (len >= 8 && len <= 20);
}

boolean registerUser(Database *db, char *username, char *password) {
    
    if (db->user_count >= MAX_USERS) {
        return false;
    }
    
    
    if (my_strlen(username) > 255 || my_strlen(username) == 0) {
        return false;
    }
    
    
    if (findUserByUsername(db, username) != NULL) {
        return false;
    }
    
    
    if (!isValidPassword(password)) {
        return false;
    }
    
    
    char new_id[MAX_USER_ID_LEN];
    generateUserID(new_id, db->user_count);
    
    
    char created_at[MAX_DATETIME_LEN];
    getCurrentDatetime(created_at, MAX_DATETIME_LEN);
    
    
    createUser(&db->users[db->user_count], new_id, username, password, 0, created_at);
    
    
    addVertex(&db->social_graph, &db->users[db->user_count]);
    
    db->user_count++;
    
    return true;
}


boolean login(Database *db, char *username, char *password) {
    User *user = findUserByUsername(db, username);
    if (user != NULL && my_strcmp(user->password, password) == 0) {
            my_strncpy(db->current_user_id, user->user_id, MAX_USER_ID_LEN - 1);
        db->current_user_id[MAX_USER_ID_LEN - 1] = '\0';
        return true;
    }
    return false;
}

void logout(Database *db) {
    db->current_user_id[0] = '\0';
}

boolean isLoggedIn(Database *db) {
    return db->current_user_id[0] != '\0';
}

char* getCurrentUserID(Database *db) {
    return db->current_user_id;
}

Post* findPostByID(Database *db, char *post_id) {
    for (int i = 0; i < db->post_count; i++) {
        if (my_strcmp(db->posts[i].post_id, post_id) == 0) {
            return &db->posts[i];
        }
    }
    return NULL;
}

Comment* findCommentByID(Database *db, int comment_id) {
    for (int i = 0; i < db->comment_count; i++) {
        if (db->comments[i].comment_id == comment_id) {
            return &db->comments[i];
        }
    }
    return NULL;
}

Subgroddit* findSubgrodditByID(Database *db, char *subgroddit_id) {
    for (int i = 0; i < db->subgroddit_count; i++) {
        if (my_strcmp(db->subgroddits[i].subgroddit_id, subgroddit_id) == 0) {
            return &db->subgroddits[i];
        }
    }
    return NULL;
}

Subgroddit* findSubgrodditByName(Database *db, char *name) {
    for (int i = 0; i < db->subgroddit_count; i++) {
        if (my_strcmp(db->subgroddits[i].name, name) == 0) {
            return &db->subgroddits[i];
        }
    }
    return NULL;
}

boolean addSubgroddit(Database *db, char *name) {
    if (!isValidSubgrodditName(name)) {
        return false;
    }
    
    if (findSubgrodditByName(db, name) != NULL) {
        return false;
    }
    
    if (db->subgroddit_count >= MAX_SUBGRODDITS) {
        return false;
    }
    
    char new_id[MAX_SUBGRODDIT_ID_LEN];
    generateSubgrodditID(new_id, db->subgroddit_count);
    
    createSubgroddit(&db->subgroddits[db->subgroddit_count], new_id, name);
    db->subgroddit_count++;
    
    return true;
}

int getPostCountBySubgroddit(Database *db, char *subgroddit_id) {
    Subgroddit *subgroddit = findSubgrodditByID(db, subgroddit_id);
    if (subgroddit != NULL) {
        return getPostCount(subgroddit);
    }
    return 0;
}

static Post* temp_posts[MAX_POSTS];

Post** getPostsBySubgroddit(Database *db, char *subgroddit_id, int *count) {
    *count = 0;
    
    Subgroddit *subgroddit = findSubgrodditByID(db, subgroddit_id);
    if (subgroddit != NULL) {
        List post_list = subgroddit->post_list;
        Address p = post_list;
        int list_index = 0;
        
        while (p != NULL && *count < MAX_POSTS) {
            int post_index = p->data;
            if (post_index >= 0 && post_index < db->post_count) {
                temp_posts[*count] = &db->posts[post_index];
                (*count)++;
            }
            p = p->next;
            list_index++;
        }
    }
    
    return temp_posts;
}

void sortPostsByHot(Post **posts, int count, boolean descending) {
    
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            int score_j = posts[j]->upvotes - posts[j]->downvotes;
            int score_j1 = posts[j + 1]->upvotes - posts[j + 1]->downvotes;
            
            boolean shouldSwap;
            if (descending) {
                shouldSwap = score_j < score_j1;
            } else {
                shouldSwap = score_j > score_j1;
            }
            
            if (shouldSwap) {
                Post *temp = posts[j];
                posts[j] = posts[j + 1];
                posts[j + 1] = temp;
            }
        }
    }
}


static int compareDatetime(const char *dt1, const char *dt2) {
    return my_strcmp(dt1, dt2);
}

void sortPostsByNew(Post **posts, int count, boolean descending) {
    
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            int cmp = compareDatetime(posts[j]->created_at, posts[j + 1]->created_at);
            
            boolean shouldSwap;
            if (descending) {
                shouldSwap = cmp < 0; 
            } else {
                shouldSwap = cmp > 0; 
            }
            
            if (shouldSwap) {
                Post *temp = posts[j];
                posts[j] = posts[j + 1];
                posts[j + 1] = temp;
            }
        }
    }
}


Vote* findVote(Database *db, char *user_id, TargetType target_type, char *target_id) {
    for (int i = 0; i < db->vote_count; i++) {
        if (my_strcmp(db->votes[i].user_id, user_id) == 0 &&
            db->votes[i].target_type == target_type &&
            my_strcmp(db->votes[i].target_id, target_id) == 0) {
            return &db->votes[i];
        }
    }
    return NULL;
}

boolean addVote(Database *db, char *user_id, TargetType target_type, char *target_id, VoteType vote_type) {
    
    if (db->vote_count >= MAX_VOTES) {
        return false;
    }
    
    
    if (findVote(db, user_id, target_type, target_id) != NULL) {
        return false;
    }
    
    
    createVote(&db->votes[db->vote_count], user_id, target_type, target_id, vote_type);
    db->vote_count++;
    
    return true;
}

boolean removeVote(Database *db, char *user_id, TargetType target_type, char *target_id) {
    for (int i = 0; i < db->vote_count; i++) {
        if (my_strcmp(db->votes[i].user_id, user_id) == 0 &&
            db->votes[i].target_type == target_type &&
            my_strcmp(db->votes[i].target_id, target_id) == 0) {
            
            for (int j = i; j < db->vote_count - 1; j++) {
                db->votes[j] = db->votes[j + 1];
            }
            db->vote_count--;
            return true;
        }
    }
    return false;
}

boolean updateVote(Database *db, char *user_id, TargetType target_type, char *target_id, VoteType new_vote_type) {
    Vote *vote = findVote(db, user_id, target_type, target_id);
    if (vote == NULL) {
        return false;
    }
    vote->vote_type = new_vote_type;
    return true;
}


Tree buildCommentTree(Database *db, char *post_id) {
    Tree root = NULL;
    
    for (int i = 0; i < db->comment_count; i++) {
        if (my_strcmp(db->comments[i].post_id, post_id) != 0) {
            continue;
        }
        
        int parent_id = db->comments[i].parent_comment_id;
        
        if (parent_id == -1) {
            
            AddressTree node = newTreeNode(&db->comments[i]);
            if (root == NULL) {
                root = node;
            } else {
                AddressTree current = root;
                while (current->nextSibling != NULL) {
                    current = current->nextSibling;
                }
                current->nextSibling = node;
            }
        } else {
            
            AddressTree parent = findNodeByCommentID(root, parent_id);
            if (parent != NULL) {
                insertChild(&parent, &db->comments[i]);
            }
        }
    }
    
    return root;
}

static char my_tolower(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c + 32;
    }
    return c;
}

static int my_strncmp_ignore_case(const char *s1, const char *s2, int n) {
    for (int i = 0; i < n; i++) {
        char c1 = my_tolower(s1[i]);
        char c2 = my_tolower(s2[i]);
        if (c1 != c2) {
            return c1 - c2;
        }
        if (c1 == '\0') {
            return 0;
        }
    }
    return 0;
}

static int is_word_boundary(char c) {
    return c == '\0' || c == ' ' || c == ',' || c == '.' || 
           c == '!' || c == '?' || c == '\n' || c == '\t' ||
           c == ';' || c == ':' || c == '"' || c == '\'';
}

boolean loadBlacklistFromCSV(Database *db, char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        db->blacklist_count = 0;
        return false;
    }
    
    STARTKATACSV(file);
    db->blacklist_count = 0;
    
    while (!EndKata && db->blacklist_count < MAX_BLACKLIST) {
        ADVKATACSV();
        if (EndKata) break;
        
        if (CKata.Length == 0) {
            continue;
        }
        
        my_strncpy(db->blacklist[db->blacklist_count], CKata.TabKata, MAX_BLACKLIST_WORD_LEN - 1);
        db->blacklist[db->blacklist_count][MAX_BLACKLIST_WORD_LEN - 1] = '\0';
        db->blacklist_count++;
    }
    
    fclose(file);
    return true;
}

char* containsBlacklistedWord(Database *db, const char *text) {
    if (text == NULL || db->blacklist_count == 0) {
        return NULL;
    }
    
    int text_len = my_strlen(text);
    
    for (int i = 0; i < db->blacklist_count; i++) {
        int word_len = my_strlen(db->blacklist[i]);
        if (word_len == 0) continue;
        
        for (int j = 0; j <= text_len - word_len; j++) {
            if (j == 0 || is_word_boundary(text[j - 1])) {
                if (my_strncmp_ignore_case(&text[j], db->blacklist[i], word_len) == 0) {
                    if (is_word_boundary(text[j + word_len])) {
                        return db->blacklist[i];
                    }
                }
            }
        }
    }
    
    return NULL;
}