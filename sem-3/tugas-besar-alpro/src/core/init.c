#include "init.h"
#include "../adt/mesinkata/mesinkata.h"
#include <stdio.h>
#include <string.h>

boolean initProgram(Database *db) {  
    while (true) {
        printf("Masukkan folder konfigurasi untuk dimuat: ");
        fflush(stdout);
        
        
        char configLine[256];
        if (fgets(configLine, sizeof(configLine), stdin) == NULL) {
            fprintf(stderr, "Error: Gagal membaca folder konfigurasi\n");
            return false;
        }
        
        
        int lineLen = 0;
        while (configLine[lineLen] != '\0' && configLine[lineLen] != '\n') {
            lineLen++;
        }
        configLine[lineLen] = '\0';
        
        
        if (lineLen == 0 || configLine[lineLen - 1] != ';') {
            printf("Error: Input harus diakhiri dengan semicolon (;)\n");
            continue;
        }
        
        
        FILE *tmpConfig = tmpfile();
        if (tmpConfig == NULL) {
            fprintf(stderr, "Error: Gagal memproses input\n");
            continue;
        }
        fprintf(tmpConfig, "%s", configLine);
        rewind(tmpConfig);
        
        STARTKATAFILE(tmpConfig);
        
        if (EndKata) {
            printf("Folder konfigurasi tidak boleh kosong!\n");
            continue;
        }
        
        
        char folderName[128];
        int i;
        for (i = 0; i < CKata.Length && i < 127; i++) {
            folderName[i] = CKata.TabKata[i];
        }
        folderName[i] = '\0';
        
        while (!EOP) {
            ADV();
        }
        
        char configDir[256];
        snprintf(configDir, sizeof(configDir), "config/%s", folderName);
        
        printf("Memuat data dari %s/...\n", configDir);
        fflush(stdout);
        
        
        char path[512];
        boolean loadSuccess = true;
        
        snprintf(path, sizeof(path), "%s/user.csv", configDir);
        if (!loadUsersFromCSV(db, path)) loadSuccess = false;
		
		snprintf(path, sizeof(path), "%s/subgroddit.csv", configDir);
        if (!loadSubgrodditsFromCSV(db, path)) loadSuccess = false;
        
        snprintf(path, sizeof(path), "%s/post.csv", configDir);
        if (!loadPostsFromCSV(db, path)) loadSuccess = false;
        
        snprintf(path, sizeof(path), "%s/comment.csv", configDir);
        if (!loadCommentsFromCSV(db, path)) loadSuccess = false;
        
        snprintf(path, sizeof(path), "%s/voting.csv", configDir);
        if (!loadVotesFromCSV(db, path)) loadSuccess = false;
        
        snprintf(path, sizeof(path), "%s/social.csv", configDir);
        if (!loadSocialsFromCSV(db, path)) loadSuccess = false;
        
        snprintf(path, sizeof(path), "%s/blacklist.csv", configDir);
        loadBlacklistFromCSV(db, path);

        if (!loadSuccess) {
            printf("Folder konfigurasi '%s' tidak ditemukan atau tidak valid. Silakan coba lagi.\n\n", folderName);
            
            createDatabase(db);
            continue;
        }
        
        strncpy(db->last_loaded_folder, configDir, 255);
        db->last_loaded_folder[255] = '\0';
        
        printf("File konfigurasi berhasil dimuat!\n");
        printf("Loaded: %d users, %d posts, %d comments, %d subgroddits, %d votes, %d socials\n", 
               db->user_count, db->post_count, db->comment_count, 
               db->subgroddit_count, db->vote_count, db->social_count);
        fflush(stdout);
        
        return true;
    }
}
