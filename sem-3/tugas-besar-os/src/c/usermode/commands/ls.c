#include "header/usermode/commands/ls.h"
#include "header/driver/ext2.h"
#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/usermode/user-shell.h"

// Helper function to navigate to a directory path and return the inode
// Returns the inode of the final directory, or 0 if path is invalid
static uint32_t resolve_path(char *path, uint32_t start_inode) {
    if (!path || strlen(path) == 0) {
        return start_inode;
    }
    
    // Handle root directory
    if (memcmp(path, "/", 1) == 0 && strlen(path) == 1) {
        return 1;  // Root inode
    }
    
    // Normalize path - remove trailing slash
    char normalized_path[512];
    uint32_t path_len = strlen(path);
    memcpy(normalized_path, path, path_len);
    normalized_path[path_len] = '\0';
    
    if (path_len > 1 && normalized_path[path_len - 1] == '/') {
        normalized_path[path_len - 1] = '\0';
        path_len--;
    }
    
    // Check if absolute path
    uint32_t working_inode = start_inode;
    char *path_to_parse = normalized_path;
    
    if (normalized_path[0] == '/') {
        working_inode = 1;  // Start from root for absolute paths
        path_to_parse = normalized_path + 1;
    }
    
    // If empty after removing leading /, it's just root
    if (strlen(path_to_parse) == 0) {
        return working_inode;
    }
    
    // Parse path components
    char path_copy[512];
    memcpy(path_copy, path_to_parse, strlen(path_to_parse));
    path_copy[strlen(path_to_parse)] = '\0';
    
    char dir_name_parts[10][256];
    uint32_t num_parts = 0;
    
    char *part_start = path_copy;
    char *part_end = path_copy;
    
    while (*part_end && num_parts < 10) {
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
            } else {
                return 0;  // Directory not found
            }
        } else {
            return 0;  // Error reading directory
        }
    }
    
    return working_inode;
}

/**
 * @brief List contents of current working directory
 * Displays all files and folders in the current directory
 * Supports paths like: ls, ls dirname, ls dir1/dir2/dir3
 * 
 * @param path Optional path to list (if NULL, list current directory)
 * @param current_inode Current working directory inode
 * @return 0 on success, error code otherwise
 */
int8_t ls_command(char *path, uint32_t current_inode) {
    uint8_t dir_data[BLOCK_SIZE * 2];
    uint32_t target_inode = current_inode;
    
    // If path is provided, resolve it to get the target directory's inode
    if (path != NULL && strlen(path) > 0) {
        target_inode = resolve_path(path, current_inode);
        if (target_inode == 0) {
            return -1;  // Path not found
        }
    }
    
    // Now read the contents of the target directory
    struct EXT2DriverRequest request = {
        .buf = dir_data,
        .parent_inode = target_inode,
        .buffer_size = BLOCK_SIZE * 2,
        .is_directory = true,
        .name = NULL,
        .name_len = 0
    };
    
    int32_t retcode = 0;
    syscall(SYS_READ_DIR, (uint32_t)&request, (uint32_t)&retcode, 0);
    
    if (retcode != 0) {
        return retcode;
    }
    
    struct EXT2DirectoryEntry *entry = (struct EXT2DirectoryEntry *)dir_data;
    char temp[256];
    uint32_t offset = 0;
    
    while (offset < BLOCK_SIZE * 2) {
        if (entry->inode == 0) break;
        
        // Get name from entry - name starts at offset 8 from entry
        char *name = (char *)((uint8_t *)entry + 8);
        
        // Validate name length - should not be 0
        if (entry->name_len == 0) {
            break;
        }
        
        // Copy name and null-terminate
        memcpy(temp, name, entry->name_len);
        temp[entry->name_len] = '\0';
        
        // Clean the string - only keep valid printable ASCII
        for (uint32_t i = 0; i < entry->name_len; i++) {
            if (temp[i] < 32 || temp[i] > 126) {
                temp[i] = '\0';
                break;
            }
        }
        
        // Print with color based on file type
        if (entry->file_type == EXT2_FT_DIR) {
            // Print folder names in red
            print_string_color(temp, COLOR_FOLDER);
        } else {
            // Print file names in default color (gray)
            print_string(temp);
        }
        print_string("\n");
        
        offset += entry->rec_len;
        if (offset < BLOCK_SIZE * 2) {
            entry = (struct EXT2DirectoryEntry *)((uint8_t *)dir_data + offset);
        } else {
            break;
        }
    }
    
    return 0;
}
