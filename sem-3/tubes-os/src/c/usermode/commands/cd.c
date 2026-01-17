#include "header/usermode/commands/cd.h"
#include "header/driver/ext2.h"
#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/usermode/user-shell.h"

// External variables for tracking current directory
extern char current_dir_name[256];
extern uint32_t current_dir_inode;
extern char current_path[512];
extern uint32_t path_depth;

/**
 * @brief Change current working directory
 * Supports absolute paths (starting with /) and .. for parent directory
 * Supports nested paths like dir1/dir2/dir3
 * 
 * @param path Target directory path
 * @param current_inode Pointer to current inode (will be updated)
 * @return 0 on success, error code otherwise
 */
int8_t cd_command(char *path, uint32_t *current_inode) {
    // If no argument, go back to root
    if (!path || strlen(path) == 0) {
        *current_inode = 1;
        current_dir_inode = 1;
        memset(current_dir_name, 0, 256);
        memset(current_path, 0, 512);
        current_path[0] = '/';
        current_path[1] = '\0';
        path_depth = 0;
        return 0;
    }
    
    uint32_t path_len = strlen(path);
    
    // Remove trailing slash if present (e.g., cd ../ becomes cd ..)
    char normalized_path[256];
    memcpy(normalized_path, path, path_len);
    normalized_path[path_len] = '\0';
    
    if (path_len > 1 && normalized_path[path_len - 1] == '/') {
        normalized_path[path_len - 1] = '\0';
        path_len--;
    }
    
    path = normalized_path;
    
    // Handle parent directory (..)
    if (memcmp(path, "..", 2) == 0 && strlen(path) == 2) {
        // Navigate to parent - find parent inode by reading current directory
        uint8_t dir_data[BLOCK_SIZE * 2];
        struct EXT2DriverRequest request = {
            .buf = dir_data,
            .parent_inode = *current_inode,
            .buffer_size = BLOCK_SIZE * 2,
            .is_directory = true
        };
        
        int32_t retcode = 0;
        syscall(SYS_READ_DIR, (uint32_t)&request, (uint32_t)&retcode, 0);
        
        if (retcode == 0) {
            struct EXT2DirectoryEntry *entry = (struct EXT2DirectoryEntry *)dir_data;
            // Skip first entry (.) and get second entry (..) which contains parent
            if (entry->rec_len > 0) {
                entry = (struct EXT2DirectoryEntry *)((uint8_t *)entry + entry->rec_len);
                if (entry->inode != 0) {
                    *current_inode = entry->inode;
                    current_dir_inode = entry->inode;
                    
                    // Update path when going to parent
                    if (entry->inode == 1) {
                        // Parent is root
                        current_dir_name[0] = '\0';
                        current_path[0] = '\0';
                        path_depth = 0;
                    } else {
                        // Remove last component from path
                        uint32_t len = strlen(current_path);
                        if (len > 0) {
                            // Find last slash
                            int32_t last_slash_pos = -1;
                            for (int32_t i = len - 1; i >= 0; i--) {
                                if (current_path[i] == '/') {
                                    last_slash_pos = i;
                                    break;
                                }
                            }
                            
                            if (last_slash_pos >= 0) {
                                // Truncate at last slash
                                current_path[last_slash_pos] = '\0';
                                
                                // Extract new current_dir_name from path
                                char *last_comp_start = last_slash_pos > 0 ? &current_path[last_slash_pos + 1] : current_path;
                                uint32_t name_len = strlen(last_comp_start);
                                memcpy(current_dir_name, last_comp_start, name_len);
                                current_dir_name[name_len] = '\0';
                                path_depth--;
                            }
                        }
                    }
                    return 0;
                }
            }
        }
        return -1;
    }
    
    // Handle root directory (/)
    if (memcmp(path, "/", 1) == 0 && strlen(path) == 1) {
        *current_inode = 1;  // Root inode is 1 in our ext2
        current_dir_inode = 1;
        current_dir_name[0] = '\0';
        current_path[0] = '\0';
        path_depth = 0;
        return 0;
    }
    
    // Handle paths with multiple levels (e.g., dir1/dir2/dir3)
    char path_copy[256];
    memcpy(path_copy, path, strlen(path));
    path_copy[strlen(path)] = '\0';
    
    // Parse path into components separated by /
    uint32_t working_inode = *current_inode;
    char dir_name_parts[10][256];  // Support up to 10 directory levels
    uint32_t num_parts = 0;
    
    // Split path by /
    char *part_start = path_copy;
    char *part_end = part_start;
    
    while (*part_end && num_parts < 10) {
        // Find next /
        while (*part_end && *part_end != '/') {
            part_end++;
        }
        
        uint32_t part_len = part_end - part_start;
        if (part_len > 0) {
            memcpy(dir_name_parts[num_parts], part_start, part_len);
            dir_name_parts[num_parts][part_len] = '\0';
            num_parts++;
        }
        
        if (*part_end == '/') {
            part_end++;
        }
        part_start = part_end;
    }
    
    // Navigate through each directory component
    for (uint32_t i = 0; i < num_parts; i++) {
        uint8_t dir_data[BLOCK_SIZE * 2];
        struct EXT2DriverRequest request = {
            .buf = dir_data,
            .name = dir_name_parts[i],
            .name_len = strlen(dir_name_parts[i]),
            .parent_inode = working_inode,
            .buffer_size = BLOCK_SIZE * 2,
            .is_directory = true
        };
        
        int32_t retcode = 0;
        syscall(SYS_READ_DIR, (uint32_t)&request, (uint32_t)&retcode, 0);
        
        if (retcode == 0) {
            struct EXT2DirectoryEntry *entry = (struct EXT2DirectoryEntry *)dir_data;
            if (entry->inode != 0) {
                working_inode = entry->inode;
                // Update current_path and current_dir_name incrementally
                uint32_t path_len = strlen(current_path);
                
                // Check if starting from root (working_inode was 1 at start of iteration)
                // and first iteration
                if (i == 0 && *current_inode == 1) {
                    // Starting from root, build path without leading slash
                    memcpy(current_path, dir_name_parts[i], strlen(dir_name_parts[i]));
                    current_path[strlen(dir_name_parts[i])] = '\0';
                } else {
                    // Add separator if not first component and not at root start
                    if (path_len > 0) {
                        current_path[path_len++] = '/';
                    }
                    
                    // Add this directory to path
                    char *comp = dir_name_parts[i];
                    uint32_t comp_len = 0;
                    while (*comp && path_len < 511) {
                        current_path[path_len++] = *comp;
                        comp++;
                        comp_len++;
                    }
                    current_path[path_len] = '\0';
                }
                
                // Update current directory name
                uint32_t comp_len = strlen(dir_name_parts[i]);
                memcpy(current_dir_name, dir_name_parts[i], comp_len);
                current_dir_name[comp_len] = '\0';
                path_depth = i + 1;
            } else {
                return -1;  // Directory not found
            }
        } else {
            return retcode;
        }
    }
    
    // Successfully navigated through all components
    *current_inode = working_inode;
    current_dir_inode = working_inode;
    
    return 0;
}
