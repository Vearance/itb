#include "header/usermode/commands/rm.h"
#include "header/driver/ext2.h"
#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/usermode/user-shell.h"

/**
 * @brief Helper to get directory inode by name
 * 
 * @param target_name Name of directory
 * @param parent_inode Parent directory inode
 * @return inode number if found, 0 otherwise
 */
uint32_t get_dir_inode(char *target_name, uint32_t parent_inode) {
    uint8_t dir_data[BLOCK_SIZE];
    struct EXT2DriverRequest request = {
        .buf = dir_data,
        .name = target_name,
        .name_len = strlen(target_name),
        .parent_inode = parent_inode,
        .buffer_size = BLOCK_SIZE,
        .is_directory = true
    };
    
    int32_t retcode = 0;
    syscall(SYS_READ_DIR, (uint32_t)&request, (uint32_t)&retcode, 0);
    
    if (retcode == 0) {
        struct EXT2DirectoryEntry *entry = (struct EXT2DirectoryEntry *)dir_data;
        return entry->inode;
    }
    
    return 0;
}

/**
 * @brief Recursively delete directory and all its contents
 * 
 * @param dir_inode Inode of directory to delete contents of
 * @return 0 on success
 */
int8_t rm_directory_recursive(uint32_t dir_inode) {
    // Keep reading and deleting until directory is empty
    while (1) {
        uint8_t dir_data[BLOCK_SIZE];
        struct EXT2DriverRequest read_request = {
            .buf = dir_data,
            .parent_inode = dir_inode,
            .buffer_size = BLOCK_SIZE,
            .is_directory = true,
            .name = NULL,
            .name_len = 0
        };
        
        int32_t retcode = 0;
        syscall(SYS_READ_DIR, (uint32_t)&read_request, (uint32_t)&retcode, 0);
        
        if (retcode != 0) {
            return retcode;
        }
        
        // Find first entry that's not . or ..
        struct EXT2DirectoryEntry *entry = (struct EXT2DirectoryEntry *)dir_data;
        uint32_t offset = 0;
        bool found = false;
        
        while (offset < BLOCK_SIZE) {
            if (entry->inode == 0 || entry->rec_len == 0) {
                break;
            }
            
            char *name = (char *)((uint8_t *)entry + 8);
            
            // Check if this is not . or ..
            if (!((entry->name_len == 1 && name[0] == '.') ||
                  (entry->name_len == 2 && name[0] == '.' && name[1] == '.'))) {
                
                // If it's a directory, recurse first
                if (entry->file_type == EXT2_FT_DIR) {
                    rm_directory_recursive(entry->inode);
                }
                
                // Delete this entry
                struct EXT2DriverRequest del_req = {
                    .buf = NULL,
                    .name = name,
                    .name_len = entry->name_len,
                    .parent_inode = dir_inode,
                    .buffer_size = 0,
                    .is_directory = (entry->file_type == EXT2_FT_DIR)
                };
                
                int32_t del_retcode = 0;
                syscall(SYS_DELETE, (uint32_t)&del_req, (uint32_t)&del_retcode, 0);
                
                found = true;
                break;  // Re-read directory after deletion
            }
            
            offset += entry->rec_len;
            if (offset < BLOCK_SIZE) {
                entry = (struct EXT2DirectoryEntry *)((uint8_t *)dir_data + offset);
            }
        }
        
        // If no entry found (except . and ..), directory is empty
        if (!found) {
            break;
        }
    }
    
    return 0;
}

/**
 * @brief Remove/delete a file or directory
 * 
 * @param target_path Path to file/directory to remove
 * @param current_inode Current working directory inode
 * @param recursive If true, delete directories recursively
 * @return 0 on success, error code otherwise
 */
int8_t rm_command(char *target_path, uint32_t current_inode, bool recursive) {
    if (!target_path || strlen(target_path) == 0) {
        return -1;
    }
    
    // First try to delete as file
    struct EXT2DriverRequest del_req = {
        .buf = NULL,
        .name = target_path,
        .name_len = strlen(target_path),
        .parent_inode = current_inode,
        .buffer_size = 0,
        .is_directory = false
    };
    
    int32_t retcode = 0;
    syscall(SYS_DELETE, (uint32_t)&del_req, (uint32_t)&retcode, 0);
    
    // If success or not found, return
    if (retcode == 0 || retcode == 1) {
        return retcode;
    }
    
    // retcode == 2 means directory not empty
    // If recursive flag is set, handle as directory
    if (recursive && retcode == 2) {
        uint32_t dir_inode = get_dir_inode(target_path, current_inode);
        
        if (dir_inode == 0) {
            return 1;  // Not found
        }
        
        // Delete all contents recursively
        rm_directory_recursive(dir_inode);
        
        // Now try to delete the (now empty) directory
        struct EXT2DriverRequest del_dir = {
            .buf = NULL,
            .name = target_path,
            .name_len = strlen(target_path),
            .parent_inode = current_inode,
            .buffer_size = 0,
            .is_directory = true
        };
        
        int32_t dir_retcode = 0;
        syscall(SYS_DELETE, (uint32_t)&del_dir, (uint32_t)&dir_retcode, 0);
        
        return dir_retcode;
    }
    
    // Return error code (2 for non-empty directory without recursive flag, 3 for invalid parent, etc)
    return retcode;
}


