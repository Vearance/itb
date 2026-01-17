#include "header/usermode/commands/find.h"
#include "header/driver/ext2.h"
#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/usermode/user-shell.h"

/**
 * @brief Recursively search directory and print full paths of matches
 */
static void find_recursive(uint32_t inode, char *target_name, uint32_t target_len, char *current_path, bool is_current_dir) {
    uint8_t dir_data[BLOCK_SIZE * 2];
    struct EXT2DriverRequest request = {
        .buf = dir_data,
        .parent_inode = inode,
        .buffer_size = BLOCK_SIZE * 2,
        .is_directory = true
    };
    
    int32_t retcode = 0;
    syscall(SYS_READ_DIR, (uint32_t)&request, (uint32_t)&retcode, 0);
    
    if (retcode != 0) return;
    
    struct EXT2DirectoryEntry *entry = (struct EXT2DirectoryEntry *)dir_data;
    uint32_t offset = 0;
    
    while (offset < BLOCK_SIZE * 2 && entry->inode != 0) {
        // Check if name matches
        char *name = (char *)((uint8_t *)entry + sizeof(struct EXT2DirectoryEntry));
        
        // Skip . and ..
        if (!((entry->name_len == 1 && name[0] == '.') ||
              (entry->name_len == 2 && name[0] == '.' && name[1] == '.'))) {
            
            // Check if filename matches (or wildcard to match all)
            bool matches = false;
            if (target_len == 1 && target_name[0] == '*') {
                // Wildcard: match all files
                matches = true;
            } else if (entry->name_len == target_len && 
                       memcmp(name, target_name, target_len) == 0) {
                // Exact match
                matches = true;
            }
            
            if (matches) {
                // Print full path with light red color (12)
                if (is_current_dir) {
                    // For current directory search, use ./<filename> format
                    print_string("./");
                } else {
                    // For other paths, use full path format
                    print_string(current_path);
                    if (current_path[strlen(current_path) - 1] != '/') {
                        print_string("/");
                    }
                }
                print_string_color(name, 12);
                print_string("\n");
            }
            
            // Recursively search subdirectories
            if (entry->file_type == EXT2_FT_DIR) {
                char new_path[512] = {0};
                uint32_t path_len = strlen(current_path);
                
                // Build new path
                memcpy(new_path, current_path, path_len);
                if (new_path[path_len - 1] != '/') {
                    new_path[path_len++] = '/';
                }
                
                // Add directory name
                uint32_t name_len = entry->name_len;
                if (path_len + name_len < 511) {
                    memcpy(&new_path[path_len], name, name_len);
                    new_path[path_len + name_len] = '\0';
                    find_recursive(entry->inode, target_name, target_len, new_path, false);
                }
            }
        }
        
        offset += entry->rec_len;
        if (offset < BLOCK_SIZE * 2) {
            entry = (struct EXT2DirectoryEntry *)((uint8_t *)dir_data + offset);
        } else {
            break;
        }
    }
}

/**
 * @brief Search for file/folder by name in specified path
 * 
 * @param search_path Path to search in ("." for current, "/" for root, or other path)
 * @param filename Name to search for
 * @param current_inode Current working directory inode
 * @return 0 on success, error code otherwise
 */
int8_t find_command(char *search_path, char *filename, uint32_t current_inode) {
    if (!search_path || !filename || strlen(filename) == 0) {
        return -1;
    }
    
    uint32_t target_len = strlen(filename);
    uint32_t start_inode = current_inode;
    char start_path[512] = {0};
    
    // Determine starting inode and path based on search_path
    if (memcmp(search_path, "/", 1) == 0 && strlen(search_path) == 1) {
        // Search from root
        start_inode = 1;
        memcpy(start_path, "/", 1);
    } else if (memcmp(search_path, ".", 1) == 0 && strlen(search_path) == 1) {
        // Search from current directory
        start_inode = current_inode;
        memcpy(start_path, ".", 1);
    } else {
        // Search from specified subdirectory
        // For now, treat as relative to current directory
        start_inode = current_inode;
        memcpy(start_path, search_path, strlen(search_path));
    }
    
    bool is_current_dir = (memcmp(search_path, ".", 1) == 0 && strlen(search_path) == 1);
    find_recursive(start_inode, filename, target_len, start_path, is_current_dir);
    
    return 0;
}
