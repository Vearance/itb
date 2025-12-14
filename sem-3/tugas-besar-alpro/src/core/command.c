#include "command.h"
#include "tui.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#define TUI_BORDER "\033[0;36m"
#define TUI_RESET "\033[0m"
#define TUI_PRIMARY "\033[1;36m"
#define TUI_SUCCESS "\033[1;32m"
#define TUI_ERROR "\033[1;31m"

static int my_strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

static size_t my_strlen(const char *s) {
    size_t len = 0;
    while (s[len] != '\0') {
        len++;
    }
    return len;
}

static char* my_strncpy(char *dest, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}


static Database *g_print_db = NULL;

static void printCommentNode(Comment *c, int indent) {
    if (c == NULL || g_print_db == NULL) {
        return;
    }

    User *author = findUserByID(g_print_db, getCommentAuthorID(c));
    char *authorName = author ? getUsername(author) : "Unknown";
    
    char author_buf[100];
    snprintf(author_buf, sizeof(author_buf), "[%d] %s", getCommentID(c), authorName);
    
    tui_print_comment_frame(indent, author_buf, getCommentContent(c), 
                           getCommentUpvotes(c), getCommentDownvotes(c), 70);
}

static char my_toLower(char c) {
    if ( c >= 'A' && c <= 'Z') {
        return c + ('a' - 'A');
    }
    return c;
}

static boolean isSubstring(const char *str, const char *substr) {
    int i, j;
    int lenStr = 0;
    int lenSub = 0;

    while (str[lenStr] != '\0') lenStr++;
    while (substr[lenSub] != '\0') lenSub++;

    if (lenSub > lenStr) {
        return false;
    }
    for (i = 0; i <= lenStr - lenSub; i++) {
        boolean match = true;
        for(j = 0; j < lenSub; j++) {
            if (my_toLower(str[i + j]) != my_toLower(substr[j])) {
                match = false;
                break;
            }
        }
        if (match) {
            return true;
        }
    }
    return false;
}

void KataToLower(Kata *K) {
    for (int i = 0; i < K->Length; i++) {
        if (K->TabKata[i] >= 'A' && K->TabKata[i] <= 'Z') {
            K->TabKata[i] = K->TabKata[i] + 32;
        }
    }
}

void handleCommand(Database *db) {
    if (EndKata) {
        return;
    }
    
    Kata cmd = CKata;
    KataToLower(&cmd);
    
    if (IsKataEqual(cmd, StringToKata("login")) || IsKataEqual(cmd, StringToKata("LOGIN"))) {
        cmdLogin(db);
    } else if (IsKataEqual(cmd, StringToKata("logout")) || IsKataEqual(cmd, StringToKata("LOGOUT"))) {
        cmdLogout(db);
    } else if (IsKataEqual(cmd, StringToKata("register")) || IsKataEqual(cmd, StringToKata("REGISTER"))) {
        cmdRegister(db);
    } else if (IsKataEqual(cmd, StringToKata("post")) || IsKataEqual(cmd, StringToKata("POST"))) {
        cmdCreatePost(db);
    } else if (IsKataEqual(cmd, StringToKata("view_post")) || IsKataEqual(cmd, StringToKata("VIEW_POST"))) {
        cmdViewPost(db);
    } else if (IsKataEqual(cmd, StringToKata("delete_post")) || IsKataEqual(cmd, StringToKata("DELETE_POST"))) {
        cmdDeletePost(db);
    } else if (IsKataEqual(cmd, StringToKata("create_comment")) || IsKataEqual(cmd, StringToKata("comment")) || IsKataEqual(cmd, StringToKata("COMMENT"))) {
        cmdCreateComment(db);
    } else if (IsKataEqual(cmd, StringToKata("delete_comment")) || IsKataEqual(cmd, StringToKata("DELETE_COMMENT"))) {
        cmdDeleteComment(db);
    } else if (IsKataEqual(cmd, StringToKata("upvote_post")) || IsKataEqual(cmd, StringToKata("UPVOTE_POST"))) {
        cmdUpvotePost(db);
    } else if (IsKataEqual(cmd, StringToKata("downvote_post")) || IsKataEqual(cmd, StringToKata("DOWNVOTE_POST"))) {
        cmdDownvotePost(db);
    } else if (IsKataEqual(cmd, StringToKata("undo_vote_post")) || IsKataEqual(cmd, StringToKata("UNDO_VOTE_POST"))) {
        cmdUndoVotePost(db);
    } else if (IsKataEqual(cmd, StringToKata("upvote_comment")) || IsKataEqual(cmd, StringToKata("UPVOTE_COMMENT"))) {
        cmdUpvoteComment(db);
    } else if (IsKataEqual(cmd, StringToKata("downvote_comment")) || IsKataEqual(cmd, StringToKata("DOWNVOTE_COMMENT"))) {
        cmdDownvoteComment(db);
    } else if (IsKataEqual(cmd, StringToKata("undo_vote_comment")) || IsKataEqual(cmd, StringToKata("UNDO_VOTE_COMMENT"))) {
        cmdUndoVoteComment(db);
    } else if (IsKataEqual(cmd, StringToKata("follow")) || IsKataEqual(cmd, StringToKata("FOLLOW")) ) {
        cmdFollow(db);
    } else if (IsKataEqual(cmd, StringToKata("unfollow")) || IsKataEqual(cmd, StringToKata("UNFOLLOW"))) {
        cmdUnfollow(db);
    } else if (IsKataEqual(cmd, StringToKata("followers")) || IsKataEqual(cmd, StringToKata("FOLLOWERS"))) {
        cmdFollowers(db);
    } else if (IsKataEqual(cmd, StringToKata("following")) || IsKataEqual(cmd, StringToKata("FOLLOWING"))) {
        cmdFollowing(db);
    } else if (IsKataEqual(cmd, StringToKata("view_profile")) || IsKataEqual(cmd, StringToKata("VIEW_PROFILE"))) {
        cmdViewProfile(db);
    } else if (IsKataEqual(cmd, StringToKata("create_subgroddit")) || IsKataEqual(cmd, StringToKata("CREATE_SUBGRODDIT"))) {
        cmdCreateSubgroddit(db);
    } else if (IsKataEqual(cmd, StringToKata("view_subgroddit")) || IsKataEqual(cmd, StringToKata("VIEW_SUBGRODDIT"))) {
        cmdViewSubgroddit(db);
    } else if (IsKataEqual(cmd, StringToKata("friend_recommendation")) || IsKataEqual(cmd, StringToKata("FRIEND_RECOMMENDATION"))) {
        cmdFriendRecommendation(db);
    } else if (IsKataEqual(cmd, StringToKata("save")) || IsKataEqual(cmd, StringToKata("SAVE"))) {
        cmdSave(db);
    } else if (IsKataEqual(cmd, StringToKata("load")) || IsKataEqual(cmd, StringToKata("LOAD"))) {
        cmdLoad(db);
    } else if (IsKataEqual(cmd, StringToKata("help")) || IsKataEqual(cmd, StringToKata("HELP"))) {
        cmdHelp();
    } else if (IsKataEqual(cmd, StringToKata("search")) || IsKataEqual(cmd, StringToKata("SEARCH"))){
        cmdSearch(db);
    } else if (IsKataEqual(cmd, StringToKata("exit")) || IsKataEqual(cmd, StringToKata("quit")) || IsKataEqual(cmd, StringToKata("EXIT")) || IsKataEqual(cmd, StringToKata("QUIT"))) {
        printf("Goodbye!\n");
        exit(0);
    } else {
        printf("Unknown command: %s. Type 'help' for available commands.\n", KataToString(cmd));
    }
}

void cmdRegister(Database *db) {
    
    if (isLoggedIn(db)) {
        User *user = findUserByID(db, getCurrentUserID(db));
        printf("Anda tidak dapat melakukan registrasi karena telah login sebagai %s\n", getUsername(user));
        return;
    }
    
    printf("Masukkan username: ");
    STARTKATA();
    if (EndKata) {
        printf("Username tidak boleh kosong.\n");
        return;
    }
    Kata username = CKata;
    char user_str[256];
    my_strncpy(user_str, KataToString(username), 255);
    user_str[255] = '\0';
    while (!EOP) {
        ADV();
    }
    
    printf("Masukkan kata sandi: ");
    STARTKATA();
    if (EndKata) {
        printf("Kata sandi tidak boleh kosong.\n");
        return;
    }
    Kata password = CKata;
    char pass_str[50];
    my_strncpy(pass_str, KataToString(password), 49);
    pass_str[49] = '\0';
    while (!EOP) {
        ADV();
    }
    
    
    if (my_strlen(user_str) > 255) {
        printf("Username terlalu panjang! Maksimal 255 karakter.\n");
        return;
    }
    
    
    int pass_len = my_strlen(pass_str);
    if (pass_len < 8) {
        printf("Kata sandi terlalu pendek! Minimal 8 karakter.\n");
        return;
    }
    if (pass_len > 20) {
        printf("Kata sandi terlalu panjang! Maksimal 20 karakter.\n");
        return;
    }
    
    
    if (findUserByUsername(db, user_str) != NULL) {
        printf("Maaf, username %s sudah terdaftar :(. Harap pilih username yang lain.\n", user_str);
        return;
    }
    
    
    if (registerUser(db, user_str, pass_str)) {
        printf("Akun dengan username %s berhasil didaftarkan! Silahkan gunakan perintah LOGIN untuk mengakses fitur-fitur Groddit\n", user_str);
    } else {
        printf("Gagal mendaftarkan akun. Silakan coba lagi.\n");
    }
}

void cmdLogin(Database *db) {
    
    if (isLoggedIn(db)) {
        User *user = findUserByID(db, getCurrentUserID(db));
        printf("Anda sudah login sebagai %s\n", getUsername(user));
        return;
    }
    
    printf("Masukkan username: ");
    STARTKATA();
    if (EndKata) {
        printf("Username tidak boleh kosong.\n");
        return;
    }
    Kata username = CKata;
    char user_str[100];
    my_strncpy(user_str, KataToString(username), 99);
    user_str[99] = '\0';
    while (!EOP) {
        ADV();
    }
    
    printf("Masukkan kata sandi: ");
    STARTKATA();
    if (EndKata) {
        printf("Kata sandi tidak boleh kosong.\n");
        return;
    }
    Kata password = CKata;
    char pass_str[100];
    my_strncpy(pass_str, KataToString(password), 99);
    pass_str[99] = '\0';
    while (!EOP) {
        ADV();
    }
    
    if (login(db, user_str, pass_str)) {
        printf("Login berhasil, selamat datang di Groddit!\n");
    } else {
        printf("Login failed! Invalid username or password.\n");
    }
}

void cmdLogout(Database *db) {
    if (!isLoggedIn(db)) {
        printf("You are not logged in.\n");
        return;
    }
    
    User *user = findUserByID(db, getCurrentUserID(db));
    printf("Cya next time, %s!\n", getUsername(user));
    logout(db);
}

void cmdViewProfile(Database *db) {
    if (!isLoggedIn(db)) {
        printf("Error: You must be logged in to view profiles.\n");
        return;
    }
    
    char username[100];
    
    ADVKATA();
    if (EndKata) {
        
        User *user = findUserByID(db, getCurrentUserID(db));
        if (user == NULL) {
            printf("Error: Current user not found.\n");
            return;
        }
        my_strncpy(username, getUsername(user), 99);
        username[99] = '\0';
    } else {
        my_strncpy(username, KataToString(CKata), 99);
        username[99] = '\0';
    }
    
    User *user = findUserByUsername(db, username);
    if (user == NULL) {
        printf("Tidak ada pengguna dengan username %s. Harap periksa masukan Anda!\n", username);
        return;
    }
    
    const char *breadcrumb_items[] = {"Home", "Profile", getUsername(user)};
    tui_print_breadcrumb(breadcrumb_items, 3);
    
    char title_buf[150];
    snprintf(title_buf, sizeof(title_buf), "USER PROFILE : %s", getUsername(user));
    tui_print_frame(title_buf, 70);
    
    char info_buf[200];
    snprintf(info_buf, sizeof(info_buf), "ID: %s", getUserID(user));
    printf(TUI_BORDER "│" TUI_RESET " %s", info_buf);
    int padding = 70 - 3 - my_strlen(info_buf);
    for (int i = 0; i < padding; i++) printf(" ");
    printf(TUI_BORDER "│\n" TUI_RESET);
    
    snprintf(info_buf, sizeof(info_buf), "Karma: %d", getKarma(user));
    printf(TUI_BORDER "│" TUI_RESET " %s", info_buf);
    padding = 70 - 3 - my_strlen(info_buf);
    for (int i = 0; i < padding; i++) printf(" ");
    printf(TUI_BORDER "│\n" TUI_RESET);
    
    snprintf(info_buf, sizeof(info_buf), "Account Created: %s", user->created_at);
    printf(TUI_BORDER "│" TUI_RESET " %s", info_buf);
    padding = 70 - 3 - my_strlen(info_buf);
    for (int i = 0; i < padding; i++) printf(" ");
    printf(TUI_BORDER "│\n" TUI_RESET);
    
    int post_count = 0;
    Post *user_posts[MAX_POSTS];
    for (int i = 0; i < db->post_count; i++) {
        if (my_strcmp(db->posts[i].author_id, getUserID(user)) == 0) {
            user_posts[post_count] = &db->posts[i];
            post_count++;
        }
    }
    
    int comment_count = 0;
    for (int i = 0; i < db->comment_count; i++) {
        if (my_strcmp(db->comments[i].author_id, getUserID(user)) == 0) {
            comment_count++;
        }
    }
    
    int followers = getInDegree(&db->social_graph, getUserID(user));
    int following = getOutDegree(&db->social_graph, getUserID(user));
    
    snprintf(info_buf, sizeof(info_buf), "Posts: %d", post_count);
    printf(TUI_BORDER "│" TUI_RESET " %s", info_buf);
    padding = 70 - 3 - my_strlen(info_buf);
    for (int i = 0; i < padding; i++) printf(" ");
    printf(TUI_BORDER "│\n" TUI_RESET);
    
    snprintf(info_buf, sizeof(info_buf), "Comments: %d", comment_count);
    printf(TUI_BORDER "│" TUI_RESET " %s", info_buf);
    padding = 70 - 3 - my_strlen(info_buf);
    for (int i = 0; i < padding; i++) printf(" ");
    printf(TUI_BORDER "│\n" TUI_RESET);
    
    snprintf(info_buf, sizeof(info_buf), "Followers: %d", followers);
    printf(TUI_BORDER "│" TUI_RESET " %s", info_buf);
    padding = 70 - 3 - my_strlen(info_buf);
    for (int i = 0; i < padding; i++) printf(" ");
    printf(TUI_BORDER "│\n" TUI_RESET);
    
    snprintf(info_buf, sizeof(info_buf), "Following: %d", following);
    printf(TUI_BORDER "│" TUI_RESET " %s", info_buf);
    padding = 70 - 3 - my_strlen(info_buf);
    for (int i = 0; i < padding; i++) printf(" ");
    printf(TUI_BORDER "│\n" TUI_RESET);
    
    User *current_user = findUserByID(db, getCurrentUserID(db));
    if (current_user != NULL && my_strcmp(getUserID(current_user), getUserID(user)) != 0) {
        if (hasEdge(&db->social_graph, getUserID(current_user), getUserID(user))) {
            snprintf(info_buf, sizeof(info_buf), "%sAnda mengikuti pengguna ini.%s", TUI_SUCCESS, TUI_RESET);
            printf(TUI_BORDER "│" TUI_RESET " %s", info_buf);
            padding = 70 - 3 - my_strlen("Anda mengikuti pengguna ini.");
            for (int i = 0; i < padding; i++) printf(" ");
            printf(TUI_BORDER "│\n" TUI_RESET);
        }
    }
    
    if (post_count > 0) {
        sortPostsByNew(user_posts, post_count, true);
        
        printf(TUI_BORDER "│" TUI_RESET " Post Terbaru:");
        padding = 70 - 3 - my_strlen(" Post Terbaru:");
        for (int i = 0; i < padding; i++) printf(" ");
        printf(TUI_BORDER "│\n" TUI_RESET);
        
        int display_count = post_count < 3 ? post_count : 3;
        for (int i = 0; i < display_count; i++) {
            Post *post = user_posts[i];
            Subgroddit *subgroddit = findSubgrodditByID(db, getPostSubgrodditID(post));
            char *subgroddit_name = subgroddit ? getSubgrodditName(subgroddit) : "Unknown";
            
            char post_buf[500];
            snprintf(post_buf, sizeof(post_buf), "  [%s] %s (%d↑) posted: %s", 
                     subgroddit_name, getPostTitle(post), 
                     getPostUpvotes(post), post->created_at);
            
            int post_len = my_strlen(post_buf);
            if (post_len > 64) {
                post_buf[64] = '\0';
                post_len = 64;
            }
            
            printf(TUI_BORDER "│" TUI_RESET " %s", post_buf);
            padding = 70 - 3 - post_len;
            for (int i = 0; i < padding; i++) printf(" ");
            printf(TUI_BORDER "│\n" TUI_RESET);
        }
    }
    
    printf(TUI_BORDER "└");
    for (int i = 0; i < 68; i++) printf("─");
    printf("┘\n" TUI_RESET);
    
    char status_left[100];
    snprintf(status_left, sizeof(status_left), "User: %s", getUsername(user));
    char status_right[50];
    snprintf(status_right, sizeof(status_right), "Karma: %d", getKarma(user));
    tui_print_status_bar(status_left, status_right);
    
    printf("\n");
}

void cmdSave(Database *db) {
    char dir[200] = "config/sample";
    
    ADVKATA();
    if (!EndKata) {
        my_strncpy(dir, KataToString(CKata), 199);
        dir[199] = '\0';
    } else {
        if (db->last_loaded_folder[0] != '\0') {
            my_strncpy(dir, db->last_loaded_folder, 199);
            dir[199] = '\0';
        } else {
            my_strncpy(dir, "sample", 199);
            dir[199] = '\0';
        }
    }
    
    const char *breadcrumb_items[] = {"Home", "Save Data"};
    tui_print_breadcrumb(breadcrumb_items, 2);
    tui_print_frame("SAVE DATA", 70);
    
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        if (mkdir(dir, 0755) != 0) {
            tui_print_error("Gagal membuat direktori!");
            printf("Lokasi: %s/\n", dir);
            return;
        }
    }
    
    char path[300];
    boolean all_success = true;
    int success_count = 0;
    
    snprintf(path, sizeof(path), "%s/user.csv", dir);
    if (saveUsersToCSV(db, path)) {
        success_count++;
    } else {
        all_success = false;
    }
    
    snprintf(path, sizeof(path), "%s/post.csv", dir);
    if (savePostsToCSV(db, path)) {
        success_count++;
    } else {
        all_success = false;
    }
    
    snprintf(path, sizeof(path), "%s/comment.csv", dir);
    if (saveCommentsToCSV(db, path)) {
        success_count++;
    } else {
        all_success = false;
    }
    
    snprintf(path, sizeof(path), "%s/subgroddit.csv", dir);
    if (saveSubgrodditsToCSV(db, path)) {
        success_count++;
    } else {
        all_success = false;
    }
    
    snprintf(path, sizeof(path), "%s/voting.csv", dir);
    if (saveVotesToCSV(db, path)) {
        success_count++;
    } else {
        all_success = false;
    }
    
    snprintf(path, sizeof(path), "%s/social.csv", dir);
    if (saveSocialsToCSV(db, path)) {
        success_count++;
    } else {
        all_success = false;
    }
    
    if (all_success) {
        tui_print_success("Data berhasil disimpan!");
        printf("Lokasi: %s/\n", dir);
        
        char status_left[256];
        snprintf(status_left, sizeof(status_left), "Saved to: %.200s", dir);
        char status_right[50];
        snprintf(status_right, sizeof(status_right), "Files: 6 CSV files");
        tui_print_status_bar(status_left, status_right);
    } else {
        tui_print_error("Beberapa file gagal disimpan!");
        printf("Lokasi: %s/\n", dir);
        printf("Berhasil: %d/6 file\n", success_count);
        
        char status_left[256];
        snprintf(status_left, sizeof(status_left), "Saved to: %.200s", dir);
        char status_right[50];
        snprintf(status_right, sizeof(status_right), "Files: %d/6 CSV files", success_count);
        tui_print_status_bar(status_left, status_right);
    }
}

void cmdLoad(Database *db) {
    char dir[200] = "config/sample";
    
    ADVKATA();
    if (!EndKata) {
        my_strncpy(dir, KataToString(CKata), 199);
        dir[199] = '\0';
    }
    
    const char *breadcrumb_items[] = {"Home", "Load Data"};
    tui_print_breadcrumb(breadcrumb_items, 2);
    tui_print_frame("LOAD DATA", 70);
    
    boolean all_success = true;
    char path[300];
    
    snprintf(path, sizeof(path), "%s/user.csv", dir);
    if (!loadUsersFromCSV(db, path)) {
        all_success = false;
    }
    
    snprintf(path, sizeof(path), "%s/post.csv", dir);
    if (!loadPostsFromCSV(db, path)) {
        all_success = false;
    }
    
    snprintf(path, sizeof(path), "%s/comment.csv", dir);
    if (!loadCommentsFromCSV(db, path)) {
        all_success = false;
    }
    
    snprintf(path, sizeof(path), "%s/subgroddit.csv", dir);
    if (!loadSubgrodditsFromCSV(db, path)) {
        all_success = false;
    }
    
    snprintf(path, sizeof(path), "%s/voting.csv", dir);
    if (!loadVotesFromCSV(db, path)) {
        all_success = false;
    }
    
    snprintf(path, sizeof(path), "%s/social.csv", dir);
    if (!loadSocialsFromCSV(db, path)) {
        all_success = false;
    }
    
    snprintf(path, sizeof(path), "%s/blacklist.csv", dir);
    loadBlacklistFromCSV(db, path);

    if (all_success) {
        my_strncpy(db->last_loaded_folder, dir, 255);
        db->last_loaded_folder[255] = '\0';
        
        tui_print_success("Data berhasil dimuat!");
        printf("Lokasi: %s/\n", dir);
        
        char status_left[256];
        snprintf(status_left, sizeof(status_left), "Loaded from: %.200s", dir);
        char status_right[80];
        snprintf(status_right, sizeof(status_right), "Users: %d | Posts: %d | Comments: %d", 
                db->user_count, db->post_count, db->comment_count);
        tui_print_status_bar(status_left, status_right);
    } else {
        tui_print_error("Beberapa file gagal dimuat!");
        printf("Lokasi: %s/\n", dir);
    }
}

void cmdCreateSubgroddit(Database *db) {
    
    if (!isLoggedIn(db)) {
        printf("Anda harus login terlebih dahulu untuk membuat Subgroddit.\n");
        return;
    }
    
    
    ADVKATA();
    if (EndKata) {
        printf("Usage: create_subgroddit <nama_subgroddit>;\n");
        printf("Contoh: create_subgroddit r/algorithms;\n");
        return;
    }
    
    char name[MAX_SUBGRODDIT_NAME_LEN];
    my_strncpy(name, KataToString(CKata), MAX_SUBGRODDIT_NAME_LEN - 1);
    name[MAX_SUBGRODDIT_NAME_LEN - 1] = '\0';
    
    
    if (!isValidSubgrodditName(name)) {
        printf("Nama Subgroddit tidak valid!\n");
        printf("Nama harus diawali dengan \"r/\" dan diikuti minimal 1 karakter.\n");
        printf("Contoh: r/algorithms, r/programming\n");
        return;
    }
    
    
    Subgroddit *existing = findSubgrodditByName(db, name);
    if (existing != NULL) {
        printf("Subgroddit %s sudah ada!\n", name);
        printf("Gunakan nama lain untuk Subgroddit baru.\n");
        return;
    }
    
    
    if (addSubgroddit(db, name)) {
        tui_print_success("Subgroddit berhasil dibuat!");
        printf("Kamu dapat mulai membuat post di dalamnya dengan perintah POST.\n");
    } else {
        printf("Gagal membuat Subgroddit. Kapasitas penuh atau terjadi kesalahan.\n");
    }
}

void cmdViewSubgroddit(Database *db) {
    
    if (!isLoggedIn(db)) {
        printf("Anda harus login terlebih dahulu untuk melihat Subgroddit.\n");
        return;
    }
    
    
    ADVKATA();
    if (EndKata) {
        printf("Usage: view_subgroddit <nama> <mode> <order>;\n");
        printf("Mode: HOT atau NEW\n");
        printf("Order: INCR atau DECR\n");
        printf("Contoh: view_subgroddit r/algorithms HOT DECR;\n");
        return;
    }
    
    char name[MAX_SUBGRODDIT_NAME_LEN];
    my_strncpy(name, KataToString(CKata), MAX_SUBGRODDIT_NAME_LEN - 1);
    name[MAX_SUBGRODDIT_NAME_LEN - 1] = '\0';
    
    
    ADVKATA();
    if (EndKata) {
        printf("Mode tidak diberikan!\n");
        printf("Gunakan HOT atau NEW sebagai mode.\n");
        return;
    }
    
    Kata modeKata = CKata;
    KataToLower(&modeKata);
    char mode[10];
    my_strncpy(mode, KataToString(modeKata), 9);
    mode[9] = '\0';
    
    
    ADVKATA();
    if (EndKata) {
        printf("Order tidak diberikan!\n");
        printf("Gunakan INCR atau DECR sebagai order.\n");
        return;
    }
    
    Kata orderKata = CKata;
    KataToLower(&orderKata);
    char order[10];
    my_strncpy(order, KataToString(orderKata), 9);
    order[9] = '\0';
    
    
    Subgroddit *subgroddit = findSubgrodditByName(db, name);
    if (subgroddit == NULL) {
        printf("Subgroddit %s belum ditemukan!\n", name);
        printf("Gunakan perintah CREATE_SUBGRODDIT untuk membuatnya terlebih dahulu.\n");
        return;
    }
    
    
    boolean isHot;
    if (my_strcmp(mode, "hot") == 0) {
        isHot = true;
    } else if (my_strcmp(mode, "new") == 0) {
        isHot = false;
    } else {
        printf("Mode %s tidak dikenali!\n", mode);
        printf("Gunakan HOT atau NEW sebagai mode yang valid.\n");
        return;
    }
    
    
    boolean isDecr;
    if (my_strcmp(order, "decr") == 0) {
        isDecr = true;
    } else if (my_strcmp(order, "incr") == 0) {
        isDecr = false;
    } else {
        printf("Order %s tidak dikenali!\n", order);
        printf("Gunakan INCR atau DECR sebagai order yang valid.\n");
        return;
    }
    
    
    int count;
    Post **posts = getPostsBySubgroddit(db, subgroddit->subgroddit_id, &count);
    
    
    if (isHot) {
        sortPostsByHot(posts, count, isDecr);
    } else {
        sortPostsByNew(posts, count, isDecr);
    }
    
    const char *breadcrumb_items[] = {"Home", "Subgroddit", name};
    tui_print_breadcrumb(breadcrumb_items, 3);
    
    char title_buf[150];
    if (isHot) {
        snprintf(title_buf, sizeof(title_buf), "Subgroddit: %s (HOT %s)", name, isDecr ? "↓" : "↑");
    } else {
        snprintf(title_buf, sizeof(title_buf), "Subgroddit: %s (NEW %s)", name, isDecr ? "↓" : "↑");
    }
    tui_print_frame(title_buf, 70);
    
    if (count == 0) {
        tui_print_info_box("Info", "Belum ada post di Subgroddit ini.");
    } else {
        for (int i = 0; i < count; i++) {
            User *author = findUserByID(db, posts[i]->author_id);
            char *authorName = author ? getUsername(author) : "Unknown";
            
            char item_buf[300];
            if (isHot) {
                snprintf(item_buf, sizeof(item_buf), "[%s] " TUI_PRIMARY "%s" TUI_RESET " (" TUI_SUCCESS "%d↑" TUI_RESET " / " TUI_ERROR "%d↓" TUI_RESET ") - oleh %s",
                        posts[i]->post_id, posts[i]->title, 
                        posts[i]->upvotes, posts[i]->downvotes, authorName);
            } else {
                snprintf(item_buf, sizeof(item_buf), "[%s] " TUI_PRIMARY "%s" TUI_RESET " (%s) - oleh %s",
                        posts[i]->post_id, posts[i]->title, 
                        posts[i]->created_at, authorName);
            }
            tui_print_list_item(i + 1, item_buf, 70);
        }
        printf(TUI_BORDER "└");
        for (int i = 0; i < 68; i++) printf("─");
        printf("┘\n" TUI_RESET);
    }
    
    if (count > 0) {
        int items_per_page = 10;
        int total_pages = (count + items_per_page - 1) / items_per_page;
        tui_print_pagination(1, total_pages, items_per_page, count);
    }
    tui_print_nav_hint("Gunakan VIEW_POST <ID> untuk melihat detail postingan.");
}

void cmdCreatePost(Database *db) {
    if (!isLoggedIn(db)) {
        printf("Anda harus login terlebih dahulu untuk membuat post.\n");
        return;
    }
    
    
    printf("Masukkan nama Subgroddit: ");
    STARTKATA();
    if (EndKata) {
        printf("Input subgroddit tidak boleh kosong.\n");
        return;
    }
    Kata subgroddit_kata = CKata;
    char subgroddit_name[MAX_SUBGRODDIT_NAME_LEN];
    my_strncpy(subgroddit_name, KataToString(subgroddit_kata), MAX_SUBGRODDIT_NAME_LEN - 1);
    subgroddit_name[MAX_SUBGRODDIT_NAME_LEN - 1] = '\0';
    while (!EOP) {
        ADV();
    }
    
    
    Subgroddit *subgroddit = findSubgrodditByName(db, subgroddit_name);
    if (subgroddit == NULL) {
        printf("Subgroddit %s belum ditemukan!\n", subgroddit_name);
        printf("Gunakan perintah CREATE_SUBGRODDIT terlebih dahulu untuk membuatnya.\n");
        return;
    }
    
    const char *breadcrumb_items[] = {"Home", "Create Post", subgroddit_name};
    tui_print_breadcrumb(breadcrumb_items, 3);
    tui_print_frame("CREATE POST", 70);
    
    printf("Masukkan judul post: ");
    STARTKATALINE();
    if (EndKata || CKata.Length == 0) {
        printf("Judul tidak boleh kosong.\n");
        return;
    }
    char title[MAX_TITLE_LEN];
    my_strncpy(title, CKata.TabKata, MAX_TITLE_LEN - 1);
    title[MAX_TITLE_LEN - 1] = '\0';
    
    
    printf("Masukkan konten post: ");
    STARTKATALINE();
    if (EndKata || CKata.Length == 0) {
        printf("Konten tidak boleh kosong.\n");
        return;
    }
    char content[MAX_CONTENT_LEN];
    my_strncpy(content, CKata.TabKata, MAX_CONTENT_LEN - 1);
    content[MAX_CONTENT_LEN - 1] = '\0'; 
    
    char *badWord = containsBlacklistedWord(db, title);
    if (badWord != NULL) {
        printf("Post gagal dibuat karena judul mengandung kata terlarang: \"%s\".\n", badWord);
        return;
    }
    
    badWord = containsBlacklistedWord(db, content);
    if (badWord != NULL) {
        printf("Post gagal dibuat karena konten mengandung kata terlarang: \"%s\".\n", badWord);
        return;
    }

    if (db->post_count >= MAX_POSTS) {
        printf("Database post sudah penuh!\n");
        return;
    }
    
    
    char post_id[MAX_POST_ID_LEN];
    snprintf(post_id, MAX_POST_ID_LEN, "P%03d", db->post_count + 1);
    
    
    char datetime[MAX_DATETIME_LEN];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    snprintf(datetime, MAX_DATETIME_LEN, "%04d-%02d-%02d %02d:%02d:%02d",
             (t->tm_year + 1900) % 10000, (t->tm_mon + 1) % 100, t->tm_mday % 100,
             t->tm_hour % 100, t->tm_min % 100, t->tm_sec % 100);
    
    
    Post *new_post = &db->posts[db->post_count];
    createPost(new_post, post_id, getSubgrodditID(subgroddit), 
               getCurrentUserID(db), title, content, datetime, 0, 0);
    
    addPostToSubgroddit(subgroddit, db->post_count);
    
    db->post_count++;
    
    tui_print_success("Post berhasil dibuat!");
    printf("ID: %s di Subgroddit %s\n", post_id, subgroddit_name);
}

void cmdViewPost(Database *db) {
    if (!isLoggedIn(db)) {
        printf("Anda harus login terlebih dahulu untuk melihat post.\n");
        return;
    }
    
    
    ADVKATA();
    if (EndKata) {
        printf("Usage: VIEW_POST [POST_ID];\n");
        return;
    }
    
    char post_id[MAX_POST_ID_LEN];
    my_strncpy(post_id, KataToString(CKata), MAX_POST_ID_LEN - 1);
    post_id[MAX_POST_ID_LEN - 1] = '\0';
    
    
    Post *post = findPostByID(db, post_id);
    if (post == NULL) {
        tui_print_error("Post tidak ditemukan!");
        return;
    }
    
    tui_loading("Memuat post");
    printf("\r");
    
    Subgroddit *subgroddit = findSubgrodditByID(db, getPostSubgrodditID(post));
    char *subgroddit_name = subgroddit ? getSubgrodditName(subgroddit) : "Unknown";
    
    User *author = findUserByID(db, getPostAuthorID(post));
    char *author_name = author ? getUsername(author) : "Unknown";
    
    char title_buf[200];
    snprintf(title_buf, sizeof(title_buf), "[%s] %s", subgroddit_name, getPostTitle(post));
    tui_print_frame(title_buf, 70);
    
    printf("\033[0;36m|\033[0m oleh: \033[1;33m%s\033[0m (%s)", author_name, post->created_at);
    int padding = 70 - 8 - my_strlen(author_name) - my_strlen(post->created_at) - 1;
    for (int i = 0; i < padding; i++) printf(" ");
    printf("\033[0;36m|\n\033[0m");
    
    tui_print_divider(70, '-');
    printf("\n%s\n\n", post->content);
    
    char vote_buf[50];
    snprintf(vote_buf, sizeof(vote_buf), "↑ %d   ↓ %d", getPostUpvotes(post), getPostDownvotes(post));
    tui_print_box(vote_buf, 70);
    printf("\n");
    
    tui_print_section("Komentar");
    Tree t = buildCommentTree(db, post_id);
    if (t == NULL) {
        tui_print_info_box("Info", "Belum ada komentar.");
    } else {
        g_print_db = db;
        printTreePreOrder(t, 0, printCommentNode);
        g_print_db = NULL;
        clearTree(&t);
    }
    
    tui_print_divider(70, '=');
    
    char status_left[100];
    snprintf(status_left, sizeof(status_left), "Post: %s | Author: %s", post_id, author_name);
    char status_right[50];
    snprintf(status_right, sizeof(status_right), "↑%d ↓%d", getPostUpvotes(post), getPostDownvotes(post));
    tui_print_status_bar(status_left, status_right);
    
    tui_print_nav_hint("Gunakan COMMENT <POST_ID> <COMMENT_ID> untuk menambahkan komentar.");
}

static void deleteCommentCascade(Database *db, char *post_id, int comment_id) {
    
    for (int i = 0; i < db->comment_count; i++) {
        if (my_strcmp(db->comments[i].post_id, post_id) == 0 &&
            db->comments[i].parent_comment_id == comment_id) {
            int child_id = db->comments[i].comment_id;
            deleteCommentCascade(db, post_id, child_id);
            
            i = -1;
        }
    }
    
    
    for (int i = 0; i < db->comment_count; i++) {
        if (db->comments[i].comment_id == comment_id &&
            my_strcmp(db->comments[i].post_id, post_id) == 0) {
            for (int j = i; j < db->comment_count - 1; j++) {
                db->comments[j] = db->comments[j + 1];
            }
            db->comment_count--;
            return;
        }
    }
}

void cmdCreateComment(Database *db) {
    if (!isLoggedIn(db)) {
        printf("Anda harus login terlebih dahulu untuk membuat komentar.\n");
        return;
    }
    
    
    ADVKATA();
    if (EndKata) {
        printf("Usage: COMMENT <POST_ID> <COMMENT_ID>;\n");
        printf("Contoh: COMMENT P001 -1;\n");
        return;
    }
    
    char post_id[MAX_POST_ID_LEN];
    my_strncpy(post_id, KataToString(CKata), MAX_POST_ID_LEN - 1);
    post_id[MAX_POST_ID_LEN - 1] = '\0';
    
    Post *post = findPostByID(db, post_id);
    if (post == NULL) {
        tui_print_error("Post tidak ditemukan!");
        return;
    }
    
    Subgroddit *subgroddit = findSubgrodditByID(db, getPostSubgrodditID(post));
    char *subgroddit_name = subgroddit ? getSubgrodditName(subgroddit) : "Unknown";
    
    const char *breadcrumb_items[] = {"Home", "Subgroddit", subgroddit_name, "Post", post_id, "Create Comment"};
    tui_print_breadcrumb(breadcrumb_items, 6);
    tui_print_frame("CREATE COMMENT", 70);
    
    
    ADVKATA();
    if (EndKata) {
        printf("Usage: COMMENT <POST_ID> <COMMENT_ID>;\n");
        printf("COMMENT_ID = -1 untuk komentar langsung ke post.\n");
        return;
    }
    
    int parent_id = 0;
    char *parent_str = KataToString(CKata);
    if (parent_str[0] == '-' && parent_str[1] == '1' && parent_str[2] == '\0') {
        parent_id = -1;
    } else {
        for (int i = 0; parent_str[i] != '\0'; i++) {
            if (parent_str[i] >= '0' && parent_str[i] <= '9') {
                parent_id = parent_id * 10 + (parent_str[i] - '0');
            }
        }
    }
    
    
    Comment *parent_comment = NULL;
    if (parent_id != -1) {
        parent_comment = findCommentByID(db, parent_id);
        if (parent_comment == NULL || my_strcmp(getCommentPostID(parent_comment), post_id) != 0) {
            printf("Komentar #%d tidak ditemukan pada post [%s]!\n", parent_id, post_id);
            return;
        }
    }
    
    
    printf("Masukkan isi komentar: ");
    STARTKATALINE();
    if (EndKata || CKata.Length == 0) {
        printf("Komentar tidak boleh kosong.\n");
        return;
    }
    
    char buffer[MAX_CONTENT_LEN];
    my_strncpy(buffer, CKata.TabKata, MAX_CONTENT_LEN - 1);
    buffer[MAX_CONTENT_LEN - 1] = '\0';
    
    int len = my_strlen(buffer);
    if (len == 0) {
        printf("Komentar tidak boleh kosong.\n");
        return;
    }
    
    char *badWord = containsBlacklistedWord(db, buffer);
    if (badWord != NULL) {
        printf("Komentar ditolak karena mengandung kata terlarang: \"%s\".\n", badWord);
        return;
    }

    if (db->comment_count >= MAX_COMMENTS) {
        printf("Database komentar sudah penuh.\n");
        return;
    }
    
    
    int new_id = 1;
    for (int i = 0; i < db->comment_count; i++) {
        if (db->comments[i].comment_id >= new_id) {
            new_id = db->comments[i].comment_id + 1;
        }
    }
    
    Comment *new_comment = &db->comments[db->comment_count];
    createComment(new_comment, new_id, post_id, getCurrentUserID(db),
                  parent_id, buffer, 0, 0);
    db->comment_count++;
    
    if (parent_id == -1) {
        printf("Komentar berhasil ditambahkan ke post %s.\n", post_id);
    } else {
        printf("Balasan berhasil ditambahkan ke komentar #%d.\n", parent_id);
    }
}

void cmdDeleteComment(Database *db) {
    if (!isLoggedIn(db)) {
        printf("Anda harus login terlebih dahulu untuk menghapus komentar.\n");
        return;
    }
    
    
    ADVKATA();
    if (EndKata) {
        printf("Usage: DELETE_COMMENT <POST_ID> <COMMENT_ID>;\n");
        return;
    }
    
    char post_id[MAX_POST_ID_LEN];
    my_strncpy(post_id, KataToString(CKata), MAX_POST_ID_LEN - 1);
    post_id[MAX_POST_ID_LEN - 1] = '\0';
    
    
    ADVKATA();
    if (EndKata) {
        printf("Usage: DELETE_COMMENT <POST_ID> <COMMENT_ID>;\n");
        return;
    }
    
    int comment_id = 0;
    char *comment_str = KataToString(CKata);
    for (int i = 0; comment_str[i] != '\0'; i++) {
        if (comment_str[i] >= '0' && comment_str[i] <= '9') {
            comment_id = comment_id * 10 + (comment_str[i] - '0');
        }
    }
    
    
    Comment *comment = findCommentByID(db, comment_id);
    if (comment == NULL || my_strcmp(getCommentPostID(comment), post_id) != 0) {
        printf("Komentar #%d tidak ditemukan pada post [%s]!\n", comment_id, post_id);
        return;
    }
    
    
    if (my_strcmp(getCommentAuthorID(comment), getCurrentUserID(db)) != 0) {
        printf("Anda hanya dapat menghapus komentar Anda sendiri.\n");
        return;
    }
    
    printf("Apakah Anda yakin ingin menghapus komentar #%d dan semua balasannya? (Y/N) ",
           comment_id);
    STARTKATA();
    if (EndKata) {
        printf("Penghapusan dibatalkan.\n");
        return;
    }
    
    Kata confirm = CKata;
    while (!EOP) {
        ADV();
    }
    KataToLower(&confirm);
    char confirm_str[10];
    my_strncpy(confirm_str, KataToString(confirm), 9);
    confirm_str[9] = '\0';
    
    if (my_strcmp(confirm_str, "y") != 0) {
        printf("Penghapusan dibatalkan.\n");
        return;
    }
    
    
    deleteCommentCascade(db, post_id, comment_id);
    
    printf("Komentar #%d dan semua reply di bawahnya berhasil dihapus.\n", comment_id);
}

void cmdDeletePost(Database *db) {
    if (!isLoggedIn(db)) {
        printf("Anda harus login terlebih dahulu untuk menghapus post.\n");
        return;
    }
    
    ADVKATA();
    if (EndKata) {
        printf("Usage: DELETE_POST [POST_ID];\n");
        return;
    }
    
    char post_id[MAX_POST_ID_LEN];
    my_strncpy(post_id, KataToString(CKata), MAX_POST_ID_LEN - 1);
    post_id[MAX_POST_ID_LEN - 1] = '\0';
    
    Post *post = findPostByID(db, post_id);
    if (post == NULL) {
        printf("Post [%s] tidak ditemukan! Anda tidak bisa menghapus fakta bahwa dia tidak dengan mu </3!!\n", post_id);
        return;
    }
    
    if (my_strcmp(getPostAuthorID(post), getCurrentUserID(db)) != 0) {
        printf("Anda bukan pembuat post [%s]!\n", post_id);
        printf("Hanya pembuat post yang dapat menghapus postingan ini.\n");
        return;
    }
    
    printf("Apakah Anda yakin ingin menghapus post [%s] \"%s\"? (Y/N) ", 
           post_id, getPostTitle(post));
    STARTKATA();
    if (EndKata) {
        printf("Penghapusan dibatalkan.\n");
        return;
    }
    
    Kata confirm = CKata;
    while (!EOP) {
        ADV();
    }
    KataToLower(&confirm);
    char confirm_str[10];
    my_strncpy(confirm_str, KataToString(confirm), 9);
    confirm_str[9] = '\0';
    
    if (my_strcmp(confirm_str, "y") != 0) {
        printf("Penghapusan dibatalkan.\n");
        return;
    }
    
    int deleted_comments = 0;
    for (int i = 0; i < db->comment_count; i++) {
        if (my_strcmp(db->comments[i].post_id, post_id) == 0) {
            for (int j = i; j < db->comment_count - 1; j++) {
                db->comments[j] = db->comments[j + 1];
            }
            db->comment_count--;
            deleted_comments++;
            i--; 
        }
    }
    
    for (int i = 0; i < db->vote_count; i++) {
        if (db->votes[i].target_type == VOTE_POST &&
            my_strcmp(db->votes[i].target_id, post_id) == 0) {
            for (int j = i; j < db->vote_count - 1; j++) {
                db->votes[j] = db->votes[j + 1];
            }
            db->vote_count--;
            i--;
        }
    }
    
    int post_index = post - db->posts;
    char *subgroddit_id = getPostSubgrodditID(post);
    Subgroddit *subgroddit = findSubgrodditByID(db, subgroddit_id);
    
    if (subgroddit != NULL) {
        removePostFromSubgroddit(subgroddit, post_index);
        for (int i = 0; i < db->subgroddit_count; i++) {
            updatePostIndices(&db->subgroddits[i], post_index);
        }
    }
    
    for (int i = post_index; i < db->post_count - 1; i++) {
        db->posts[i] = db->posts[i + 1];
    }
    db->post_count--;
    
    printf("\nPost [%s] dan seluruh komentar terkait berhasil dihapus.\n", post_id);
}

void cmdFollow(Database *db) {
    if (!isLoggedIn(db)) {
        printf("Anda harus login terlebih dahulu untuk mengikuti pengguna lain.\n");
        return;
    }

    ADVKATA();
    if (EndKata) {
        printf("Usage: follow <username>;\n");
        return;
    }

    char target[100];
    my_strncpy(target, KataToString(CKata), 99);
    target[99] = '\0';

    User *targetUser = findUserByUsername(db, target);
    if (targetUser == NULL) {
        printf("Pengguna %s tidak ditemukan.\n", target);
        return;
    }

    User *current = findUserByID(db, getCurrentUserID(db));

    if (my_strcmp(getUserID(current), getUserID(targetUser)) == 0) {
        printf("Anda tidak dapat mengikuti diri sendiri.\n");
        return;
    }

    if (hasEdge(&db->social_graph, getUserID(current), getUserID(targetUser))) {
        printf("Anda sudah mengikuti %s.\n", getUsername(targetUser));
        return;
    }

    if (db->social_count >= MAX_SOCIALS) {
        printf("Database social sudah penuh!\n");
        return;
    }

    addEdge(&db->social_graph, getUserID(current), getUserID(targetUser));
    
    createSocial(&db->socials[db->social_count], getUserID(current), getUserID(targetUser));
    db->social_count++;

    printf("Anda berhasil mengikuti %s.\n", getUsername(targetUser));
}

void cmdUnfollow(Database *db) {
    if (!isLoggedIn(db)) {
        printf("Anda harus login terlebih dahulu untuk berhenti mengikuti pengguna.\n");
        return;
    }

    ADVKATA();
    if (EndKata) {
        printf("Usage: unfollow <username>;\n");
        return;
    }

    char target[100];
    my_strncpy(target, KataToString(CKata), 99);
    target[99] = '\0';

    User *targetUser = findUserByUsername(db, target);
    if (targetUser == NULL) {
        printf("Pengguna %s tidak ditemukan.\n", target);
        return;
    }

    User *current = findUserByID(db, getCurrentUserID(db));

    if (!hasEdge(&db->social_graph, getUserID(current), getUserID(targetUser))) {
        printf("Anda belum mengikuti %s.\n", getUsername(targetUser));
        return;
    }

    removeEdge(&db->social_graph, getUserID(current), getUserID(targetUser));
    
    for (int i = 0; i < db->social_count; i++) {
        if (my_strcmp(db->socials[i].follower_id, getUserID(current)) == 0 &&
            my_strcmp(db->socials[i].following_id, getUserID(targetUser)) == 0) {
            // shift array
            for (int j = i; j < db->social_count - 1; j++) {
                db->socials[j] = db->socials[j + 1];
            }
            db->social_count--;
            break;
        }
    }

    printf("Anda berhasil berhenti mengikuti %s.\n", getUsername(targetUser));
}

void cmdFollowers(Database *db) {
    if (!isLoggedIn(db)) {
        printf("Anda harus login terlebih dahulu untuk melihat followers.\n");
        return;
    }

    ADVKATA();
    if (EndKata) {
        printf("Usage: followers <username>;\n");
        return;
    }

    char target[100];
    my_strncpy(target, KataToString(CKata), 99);
    target[99] = '\0';

    User *targetUser = findUserByUsername(db, target);
    if (targetUser == NULL) {
        tui_print_error("User tidak ditemukan!");
        return;
    }

    const char *breadcrumb_items[] = {"Home", "Profile", getUsername(targetUser), "Followers"};
    tui_print_breadcrumb(breadcrumb_items, 4);

    char title_buf[150];
    snprintf(title_buf, sizeof(title_buf), "Pengikut %s", getUsername(targetUser));
    tui_print_frame(title_buf, 70);

    int count = 0;
    for (int i = 0; i < db->user_count; i++) {
        User *u = &db->users[i];
        if (hasEdge(&db->social_graph, getUserID(u), getUserID(targetUser))) {
            count++;
            tui_print_list_item(count, getUsername(u), 70);
        }
    }

    if (count == 0) {
        tui_print_info_box("Info", "Tidak ada pengikut");
    } else {
        printf(TUI_BORDER "└");
        for (int i = 0; i < 68; i++) printf("-");
        printf("┘\n" TUI_RESET);
        
        if (count > 10) {
            int items_per_page = 10;
            int total_pages = (count + items_per_page - 1) / items_per_page;
            tui_print_pagination(1, total_pages, items_per_page, count);
        }
    }
    tui_print_nav_hint("Gunakan VIEW_PROFILE <username> untuk melihat profil user.");
    printf("\n");
}

void cmdFollowing(Database *db) {
    if (!isLoggedIn(db)) {
        printf("Anda harus login terlebih dahulu untuk melihat following.\n");
        return;
    }

    ADVKATA();
    if (EndKata) {
        printf("Usage: following <username>;\n");
        return;
    }

    char target[100];
    my_strncpy(target, KataToString(CKata), 99);
    target[99] = '\0';

    User *targetUser = findUserByUsername(db, target);
    if (targetUser == NULL) {
        tui_print_error("User tidak ditemukan!");
        return;
    }

    const char *breadcrumb_items[] = {"Home", "Profile", getUsername(targetUser), "Following"};
    tui_print_breadcrumb(breadcrumb_items, 4);

    char title_buf[150];
    snprintf(title_buf, sizeof(title_buf), "%s mengikuti", getUsername(targetUser));
    tui_print_frame(title_buf, 70);

    int count = 0;
    for (int i = 0; i < db->user_count; i++) {
        User *u = &db->users[i];
        if (hasEdge(&db->social_graph, getUserID(targetUser), getUserID(u))) {
            count++;
            tui_print_list_item(count, getUsername(u), 70);
        }
    }

    if (count == 0) {
        tui_print_info_box("Info", "Tidak mengikuti siapa pun");
    } else {
        printf(TUI_BORDER "└");
        for (int i = 0; i < 68; i++) printf("─");
        printf("┘\n" TUI_RESET);
    }
    printf("\n");
}

void cmdUpvotePost(Database *db) {
    if (!isLoggedIn(db)) {
        printf("Anda harus login terlebih dahulu untuk memberikan vote.\n");
        return;
    }
    
    
    ADVKATA();
    if (EndKata) {
        printf("Usage: UPVOTE_POST <POST_ID>;\n");
        return;
    }
    
    char post_id[MAX_POST_ID_LEN];
    my_strncpy(post_id, KataToString(CKata), MAX_POST_ID_LEN - 1);
    post_id[MAX_POST_ID_LEN - 1] = '\0';
    
    
    Post *post = findPostByID(db, post_id);
    if (post == NULL) {
        printf("Post dengan ID %s tidak ditemukan!\n", post_id);
        return;
    }
    
    
    if (my_strcmp(getPostAuthorID(post), getCurrentUserID(db)) == 0) {
        printf("Anda tidak dapat memberikan vote pada post Anda sendiri!\n");
        return;
    }
    
    
    User *post_owner = findUserByID(db, getPostAuthorID(post));
    
    
    Vote *exist = findVote(db, getCurrentUserID(db), VOTE_POST, post_id);
    
    if (exist != NULL) {
        if (exist->vote_type == VOTE_UP) {
            printf("Anda sudah memberikan upvote pada post [%s].\n", post_id);
            return;
        } else {
            
            exist->vote_type = VOTE_UP;
            setPostDownvotes(post, getPostDownvotes(post) - 1);
            setPostUpvotes(post, getPostUpvotes(post) + 1);
            if (post_owner != NULL) {
                setKarma(post_owner, getKarma(post_owner) + 2); 
            }
            printf("Vote Anda pada post [%s] berhasil diubah menjadi upvote.\n", post_id);
            return;
        }
    }
    
    
    if (addVote(db, getCurrentUserID(db), VOTE_POST, post_id, VOTE_UP)) {
        setPostUpvotes(post, getPostUpvotes(post) + 1);
        if (post_owner != NULL) {
            setKarma(post_owner, getKarma(post_owner) + 1);
        }
        printf("Anda berhasil memberikan upvote pada post [%s].\n", post_id);
    } else {
        printf("Gagal memberikan vote. Silakan coba lagi.\n");
    }
}

void cmdDownvotePost(Database *db) {
    if (!isLoggedIn(db)) {
        printf("Anda harus login terlebih dahulu untuk memberikan vote.\n");
        return;
    }
    
    
    ADVKATA();
    if (EndKata) {
        printf("Usage: DOWNVOTE_POST <POST_ID>;\n");
        return;
    }
    
    char post_id[MAX_POST_ID_LEN];
    my_strncpy(post_id, KataToString(CKata), MAX_POST_ID_LEN - 1);
    post_id[MAX_POST_ID_LEN - 1] = '\0';
    
    
    Post *post = findPostByID(db, post_id);
    if (post == NULL) {
        printf("Post dengan ID %s tidak ditemukan!\n", post_id);
        return;
    }
    
    
    if (my_strcmp(getPostAuthorID(post), getCurrentUserID(db)) == 0) {
        printf("Anda tidak dapat memberikan vote pada post Anda sendiri!\n");
        return;
    }
    
    
    User *post_owner = findUserByID(db, getPostAuthorID(post));
    
    
    Vote *exist = findVote(db, getCurrentUserID(db), VOTE_POST, post_id);
    
    if (exist != NULL) {
        if (exist->vote_type == VOTE_DOWN) {
            printf("Anda sudah memberikan downvote pada post [%s].\n", post_id);
            return;
        } else {
            
            exist->vote_type = VOTE_DOWN;
            setPostUpvotes(post, getPostUpvotes(post) - 1);
            setPostDownvotes(post, getPostDownvotes(post) + 1);
            if (post_owner != NULL) {
                setKarma(post_owner, getKarma(post_owner) - 2); 
            }
            printf("Vote Anda pada post [%s] berhasil diubah menjadi downvote.\n", post_id);
            return;
        }
    }
    
    
    if (addVote(db, getCurrentUserID(db), VOTE_POST, post_id, VOTE_DOWN)) {
        setPostDownvotes(post, getPostDownvotes(post) + 1);
        if (post_owner != NULL) {
            setKarma(post_owner, getKarma(post_owner) - 1);
        }
        printf("Anda berhasil memberikan downvote pada post [%s].\n", post_id);
    } else {
        printf("Gagal memberikan vote. Silakan coba lagi.\n");
    }
}

void cmdUndoVotePost(Database *db) {
    if (!isLoggedIn(db)) {
        printf("Anda harus login terlebih dahulu untuk membatalkan vote.\n");
        return;
    }
    
    
    ADVKATA();
    if (EndKata) {
        printf("Usage: UNDO_VOTE_POST <POST_ID>;\n");
        return;
    }
    
    char post_id[MAX_POST_ID_LEN];
    my_strncpy(post_id, KataToString(CKata), MAX_POST_ID_LEN - 1);
    post_id[MAX_POST_ID_LEN - 1] = '\0';
    
    
    Post *post = findPostByID(db, post_id);
    if (post == NULL) {
        printf("Post dengan ID %s tidak ditemukan!\n", post_id);
        return;
    }
    
    
    Vote *exist = findVote(db, getCurrentUserID(db), VOTE_POST, post_id);
    
    if (exist == NULL) {
        printf("Anda belum memberikan vote pada post [%s].\n", post_id);
        return;
    }
    
    
    User *post_owner = findUserByID(db, getPostAuthorID(post));
    
    
    VoteType old_vote_type = exist->vote_type;
    if (removeVote(db, getCurrentUserID(db), VOTE_POST, post_id)) {
        if (old_vote_type == VOTE_UP) {
            setPostUpvotes(post, getPostUpvotes(post) - 1);
            if (post_owner != NULL) {
                setKarma(post_owner, getKarma(post_owner) - 1);
            }
        } else {
            setPostDownvotes(post, getPostDownvotes(post) - 1);
            if (post_owner != NULL) {
                setKarma(post_owner, getKarma(post_owner) + 1);
            }
        }
        printf("Vote Anda pada post [%s] berhasil dibatalkan.\n", post_id);
    } else {
        printf("Gagal membatalkan vote. Silakan coba lagi.\n");
    }
}

void cmdUpvoteComment(Database *db) {
    if (!isLoggedIn(db)) {
        printf("Anda harus login terlebih dahulu untuk memberikan vote.\n");
        return;
    }
    
    
    ADVKATA();
    if (EndKata) {
        printf("Usage: UPVOTE_COMMENT <POST_ID> <COMMENT_ID>;\n");
        return;
    }
    
    char post_id[MAX_POST_ID_LEN];
    my_strncpy(post_id, KataToString(CKata), MAX_POST_ID_LEN - 1);
    post_id[MAX_POST_ID_LEN - 1] = '\0';
    
    
    Post *post = findPostByID(db, post_id);
    if (post == NULL) {
        printf("Post dengan ID %s tidak ditemukan!\n", post_id);
        return;
    }
    
    
    ADVKATA();
    if (EndKata) {
        printf("Usage: UPVOTE_COMMENT <POST_ID> <COMMENT_ID>;\n");
        return;
    }
    
    int comment_id = 0;
    char *comment_id_str = KataToString(CKata);
    for (int i = 0; comment_id_str[i] != '\0'; i++) {
        if (comment_id_str[i] >= '0' && comment_id_str[i] <= '9') {
            comment_id = comment_id * 10 + (comment_id_str[i] - '0');
        }
    }
    
    
    Comment *comment = findCommentByID(db, comment_id);
    if (comment == NULL || my_strcmp(getCommentPostID(comment), post_id) != 0) {
        printf("Komentar #%d tidak ditemukan pada post [%s]!\n", comment_id, post_id);
        return;
    }
    
    
    if (my_strcmp(getCommentAuthorID(comment), getCurrentUserID(db)) == 0) {
        printf("Anda tidak dapat memberikan vote pada komentar Anda sendiri!\n");
        return;
    }
    
    
    User *comment_owner = findUserByID(db, getCommentAuthorID(comment));
    
    
    char target_id[20];
    snprintf(target_id, sizeof(target_id), "%d", comment_id);
    
    
    Vote *exist = findVote(db, getCurrentUserID(db), VOTE_COMMENT, target_id);
    
    if (exist != NULL) {
        if (exist->vote_type == VOTE_UP) {
            printf("Anda sudah memberikan upvote pada komentar #%d di post [%s].\n", comment_id, post_id);
            return;
        } else {
            
            exist->vote_type = VOTE_UP;
            setCommentDownvotes(comment, getCommentDownvotes(comment) - 1);
            setCommentUpvotes(comment, getCommentUpvotes(comment) + 1);
            if (comment_owner != NULL) {
                setKarma(comment_owner, getKarma(comment_owner) + 2);
            }
            printf("Vote Anda pada komentar #%d di post [%s] berhasil diubah menjadi upvote.\n", comment_id, post_id);
            return;
        }
    }
    
    
    if (addVote(db, getCurrentUserID(db), VOTE_COMMENT, target_id, VOTE_UP)) {
        setCommentUpvotes(comment, getCommentUpvotes(comment) + 1);
        if (comment_owner != NULL) {
            setKarma(comment_owner, getKarma(comment_owner) + 1);
        }
        printf("Anda berhasil memberikan upvote pada komentar #%d di post [%s].\n", comment_id, post_id);
    } else {
        printf("Gagal memberikan vote. Silakan coba lagi.\n");
    }
}

void cmdDownvoteComment(Database *db) {
    if (!isLoggedIn(db)) {
        printf("Anda harus login terlebih dahulu untuk memberikan vote.\n");
        return;
    }
    
    
    ADVKATA();
    if (EndKata) {
        printf("Usage: DOWNVOTE_COMMENT <POST_ID> <COMMENT_ID>;\n");
        return;
    }
    
    char post_id[MAX_POST_ID_LEN];
    my_strncpy(post_id, KataToString(CKata), MAX_POST_ID_LEN - 1);
    post_id[MAX_POST_ID_LEN - 1] = '\0';
    
    
    Post *post = findPostByID(db, post_id);
    if (post == NULL) {
        printf("Post dengan ID %s tidak ditemukan!\n", post_id);
        return;
    }
    
    
    ADVKATA();
    if (EndKata) {
        printf("Usage: DOWNVOTE_COMMENT <POST_ID> <COMMENT_ID>;\n");
        return;
    }
    
    int comment_id = 0;
    char *comment_id_str = KataToString(CKata);
    for (int i = 0; comment_id_str[i] != '\0'; i++) {
        if (comment_id_str[i] >= '0' && comment_id_str[i] <= '9') {
            comment_id = comment_id * 10 + (comment_id_str[i] - '0');
        }
    }
    
    
    Comment *comment = findCommentByID(db, comment_id);
    if (comment == NULL || my_strcmp(getCommentPostID(comment), post_id) != 0) {
        printf("Komentar #%d tidak ditemukan pada post [%s]!\n", comment_id, post_id);
        return;
    }
    
    
    if (my_strcmp(getCommentAuthorID(comment), getCurrentUserID(db)) == 0) {
        printf("Anda tidak dapat memberikan vote pada komentar Anda sendiri!\n");
        return;
    }
    
    
    User *comment_owner = findUserByID(db, getCommentAuthorID(comment));
    
    
    char target_id[20];
    snprintf(target_id, sizeof(target_id), "%d", comment_id);
    
    
    Vote *exist = findVote(db, getCurrentUserID(db), VOTE_COMMENT, target_id);
    
    if (exist != NULL) {
        if (exist->vote_type == VOTE_DOWN) {
            printf("Anda sudah memberikan downvote pada komentar #%d di post [%s].\n", comment_id, post_id);
            return;
        } else {
            
            exist->vote_type = VOTE_DOWN;
            setCommentUpvotes(comment, getCommentUpvotes(comment) - 1);
            setCommentDownvotes(comment, getCommentDownvotes(comment) + 1);
            if (comment_owner != NULL) {
                setKarma(comment_owner, getKarma(comment_owner) - 2);
            }
            printf("Vote Anda pada komentar #%d di post [%s] berhasil diubah menjadi downvote.\n", comment_id, post_id);
            return;
        }
    }
    
    
    if (addVote(db, getCurrentUserID(db), VOTE_COMMENT, target_id, VOTE_DOWN)) {
        setCommentDownvotes(comment, getCommentDownvotes(comment) + 1);
        if (comment_owner != NULL) {
            setKarma(comment_owner, getKarma(comment_owner) - 1);
        }
        printf("Anda berhasil memberikan downvote pada komentar #%d di post [%s].\n", comment_id, post_id);
    } else {
        printf("Gagal memberikan vote. Silakan coba lagi.\n");
    }
}

void cmdUndoVoteComment(Database *db) {
    if (!isLoggedIn(db)) {
        printf("Anda harus login terlebih dahulu untuk membatalkan vote.\n");
        return;
    }
    
    
    ADVKATA();
    if (EndKata) {
        printf("Usage: UNDO_VOTE_COMMENT <POST_ID> <COMMENT_ID>;\n");
        return;
    }
    
    char post_id[MAX_POST_ID_LEN];
    my_strncpy(post_id, KataToString(CKata), MAX_POST_ID_LEN - 1);
    post_id[MAX_POST_ID_LEN - 1] = '\0';
    
    
    Post *post = findPostByID(db, post_id);
    if (post == NULL) {
        printf("Post dengan ID %s tidak ditemukan!\n", post_id);
        return;
    }
    
    
    ADVKATA();
    if (EndKata) {
        printf("Usage: UNDO_VOTE_COMMENT <POST_ID> <COMMENT_ID>;\n");
        return;
    }
    
    int comment_id = 0;
    char *comment_id_str = KataToString(CKata);
    for (int i = 0; comment_id_str[i] != '\0'; i++) {
        if (comment_id_str[i] >= '0' && comment_id_str[i] <= '9') {
            comment_id = comment_id * 10 + (comment_id_str[i] - '0');
        }
    }
    
    
    Comment *comment = findCommentByID(db, comment_id);
    if (comment == NULL || my_strcmp(getCommentPostID(comment), post_id) != 0) {
        printf("Komentar #%d tidak ditemukan pada post [%s]!\n", comment_id, post_id);
        return;
    }
    
    
    char target_id[20];
    snprintf(target_id, sizeof(target_id), "%d", comment_id);
    
    
    Vote *exist = findVote(db, getCurrentUserID(db), VOTE_COMMENT, target_id);
    
    if (exist == NULL) {
        printf("Anda belum memberikan vote pada komentar #%d di post [%s].\n", comment_id, post_id);
        return;
    }
    
    
    User *comment_owner = findUserByID(db, getCommentAuthorID(comment));
    
    
    VoteType old_vote_type = exist->vote_type;
    if (removeVote(db, getCurrentUserID(db), VOTE_COMMENT, target_id)) {
        if (old_vote_type == VOTE_UP) {
            setCommentUpvotes(comment, getCommentUpvotes(comment) - 1);
            if (comment_owner != NULL) {
                setKarma(comment_owner, getKarma(comment_owner) - 1);
            }
        } else {
            setCommentDownvotes(comment, getCommentDownvotes(comment) - 1);
            if (comment_owner != NULL) {
                setKarma(comment_owner, getKarma(comment_owner) + 1);
            }
        }
        printf("Vote Anda pada komentar #%d di post [%s] berhasil dibatalkan.\n", comment_id, post_id);
    } else {
        printf("Gagal membatalkan vote. Silakan coba lagi.\n");
    }
}


void cmdSearch(Database *db) {
    char query[100];
    int i;
    int foundCount = 0;

    ADVKATA();

    if (EndKata) {
        tui_print_error("Error: Masukkan kata kunci pencarian. Contoh: search algoritma");
        return;
    }

    int queryIdx = 0;
    while (!EndKata) {
        char *word = KataToString(CKata);
        int wordLen = KataLength(CKata);

        if (queryIdx + wordLen + 1 >= 100) {
            break;
        }
        if (queryIdx > 0) {
            query[queryIdx] = ' ';
            queryIdx++;
        }

        for (int k = 0; k < wordLen; k++) {
            query[queryIdx++] = word[k];
        }
        ADVKATA();
    }
    query[queryIdx] = '\0';
    printf("Mencari \"%s\" di seluruh database...\n\n", query);

    tui_print_section ("Subgroddits Found");
    boolean foundSub = false;
    
    for(i = 0; i < db->subgroddit_count; i++) {
        if (isSubstring(db->subgroddits[i].name, query)) {
            printf("- %s (ID: %s)\n", db->subgroddits[i].name, db->subgroddits[i].subgroddit_id);
            foundSub = true;
            foundCount++;
        }
    }

    if ( !foundSub ) {
        printf("Tidak ada Subgroddit yang cocok.\n");
    }

    tui_print_section("Users Found");
    boolean foundUser = false;
    for (i = 0; i < db->user_count; i++) {
        if(isSubstring(db->users[i].username, query)) {
            printf("- u/%s (Karma: %d)\n", db->users[i].username, db->users[i].karma);
            foundUser = true;
            foundCount++;
        }
    }
    
    if (!foundUser) {
        printf("Tidak ada User yang cocok. \n");
    }

    tui_print_section("Posts Found");
    boolean foundPost = false;
    for (i = 0; i < db->post_count; i++) {
        if(isSubstring(db->posts[i].title, query) || isSubstring(db->posts[i].content, query)) {
            char *author = db->posts[i].author_id;
            printf("[%s] %s\n", db->posts[i].post_id, db->posts[i].title);
            printf("       Author: %s | Date: %s\n", author, db->posts[i].created_at);

            char preview[51];
            int len = 0;
            while(db->posts[i].content[len] != '\0' && len < 50) {
                preview[len] = db->posts[i].content[len];
                len++;
            }
            
            preview[len] = '\0';
            printf("        \"%s...\"\n\n", preview);

            foundPost = true;
            foundCount++;
        }
    }
    if (!foundPost) {
        printf("Tidak ada Post yang cocok.\n");
    }
    printf("\nTotal hasil ditemukan: %d\n", foundCount);
}


void cmdFriendRecommendation(Database *db) {
    if (!isLoggedIn(db)) {
        printf("Anda harus login terlebih dahulu untuk melihat rekomendasi teman.\n");
        return;
    }
    
    User *currentUser = findUserByID(db, getCurrentUserID(db));
    if (currentUser == NULL) {
        printf("Error: User tidak ditemukan.\n");
        return;
    }
    
    const char *breadcrumb_items[] = {"Home", "Friend Recommendation"};
    tui_print_breadcrumb(breadcrumb_items, 2);
    
    char title_buf[100];
    snprintf(title_buf, sizeof(title_buf), "Rekomendasi Teman untuk %s", getUsername(currentUser));
    tui_print_frame(title_buf, 60);
    
    FriendRecommendation recommendations[10];
    int count = 0;
    getRecommendations(&db->social_graph, getCurrentUserID(db), recommendations, &count, 10);
    
    if (count == 0) {
        printf("Tidak ada rekomendasi teman saat ini.\n");
    } else {
        printf("Menampilkan rekomendasi teman untuk pengguna...\n\n");
        for (int i = 0; i < count; i++) {
            User *recommendedUser = db->social_graph.vertices[recommendations[i].userIndex];
            if (recommendedUser != NULL) {
                printf("%d. %s (%d mutual connections)\n", 
                       i + 1, 
                       getUsername(recommendedUser), 
                       recommendations[i].mutualCount);
            }
        }
    }
    
    printf("\n");
    tui_print_divider(60, '-');
    printf("Gunakan FOLLOW <username> untuk mengikuti pengguna.\n");
}

void cmdHelp() {
    const char *breadcrumb_items[] = {"Home", "Help"};
    tui_print_breadcrumb(breadcrumb_items, 2);
    
    tui_print_section("Groddit Commands");
    printf("register;                                       - Register a new account\n");
    printf("login;                                          - Login to the system\n");
    printf("logout;                                         - Logout from the system\n");
    printf("post;                                           - Create a new post\n");
    printf("view_post <post_id>;                            - View a post and its comments\n");
    printf("delete_post <post_id>;                          - Delete your own post\n");
    printf("comment <post_id> <comment_id>;                 - Create a comment/reply (-1 for top-level)\n");
    printf("delete_comment <post_id> <comment_id>;          - Delete your own comment (cascading)\n");
    tui_print_section("Vote Commands");
    printf("upvote_post <post_id>;                          - Upvote a post\n");
    printf("downvote_post <post_id>;                        - Downvote a post\n");
    printf("undo_vote_post <post_id>;                       - Undo your vote on a post\n");
    printf("upvote_comment <post_id> <comment_id>;          - Upvote a comment\n");
    printf("downvote_comment <post_id> <comment_id>;        - Downvote a comment\n");
    printf("undo_vote_comment <post_id> <comment_id>;       - Undo your vote on a comment\n");
    tui_print_section("Social Commands");
    printf("follow <username>;                              - Follow a user\n");
    printf("unfollow <username>;                            - Unfollow a user\n");
    printf("followers <username>;                           - View followers\n");
    printf("following <username>;                           - View following\n");
    printf("view_profile <username>;                        - View user profile\n");
    printf("friend_recommendation;                          - Get friend recommendations\n");
    tui_print_section("Subgroddit Commands");
    printf("create_subgroddit <nama>;                       - Create a new subgroddit (e.g. r/algorithms)\n");
    printf("view_subgroddit <nama> <mode> <order>;          - View posts in a subgroddit\n");
    printf("   Mode: HOT (by votes) or NEW (by date)\n");
    printf("   Order: INCR (ascending) or DECR (descending)\n");
    tui_print_section("Data Commands");
    printf("save <directory>;                               - Save data to CSV files\n");
    printf("load <directory>;                               - Load data from CSV files\n");
    tui_print_section("Others");
    printf("search <keyword>;                               - Search users, posts, subgroddits\n");
    printf("help;                                           - Show this help message\n"); 
    printf("exit; / quit;                                   - Exit the program\n");
    printf("\nNote: All commands must end with semicolon (;)\n");
    
    char status_left[100];
    snprintf(status_left, sizeof(status_left), "Groddit Help System");
    char status_right[50];
    snprintf(status_right, sizeof(status_right), "Type command; to execute");
    tui_print_status_bar(status_left, status_right);
    
    printf("\n");
}