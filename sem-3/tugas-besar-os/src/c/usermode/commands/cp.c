#include "header/usermode/commands/cp.h"
#include "header/driver/ext2.h"
#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/usermode/user-shell.h"

#define CP_BUFFER_SIZE (BLOCK_SIZE * 8)

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

// Helper function to extract filename from path
static void extract_filename(char *path, char *filename) {
    int32_t last_slash = -1;
    uint32_t path_len = strlen(path);
    
    // Remove trailing slash if present
    if (path_len > 0 && path[path_len - 1] == '/') {
        path_len--;
    }
    
    // Find last slash
    for (int32_t i = path_len - 1; i >= 0; i--) {
        if (path[i] == '/') {
            last_slash = i;
            break;
        }
    }
    
    if (last_slash >= 0) {
        // Copy everything after last slash
        uint32_t name_len = path_len - last_slash - 1;
        memcpy(filename, &path[last_slash + 1], name_len);
        filename[name_len] = '\0';
    } else {
        // No slash, entire path is filename
        memcpy(filename, path, path_len);
        filename[path_len] = '\0';
    }
}

// Helper function to extract directory path from full path
static void extract_directory(char *path, char *directory) {
    uint32_t path_len = strlen(path);
    
    // Remove trailing slash if present
    if (path_len > 0 && path[path_len - 1] == '/') {
        path_len--;
    }
    
    // Find last slash
    int32_t last_slash = -1;
    for (int32_t i = path_len - 1; i >= 0; i--) {
        if (path[i] == '/') {
            last_slash = i;
            break;
        }
    }
    
    if (last_slash >= 0) {
        // Copy everything up to and including the last slash
        memcpy(directory, path, last_slash + 1);
        directory[last_slash + 1] = '\0';
    } else {
        // No slash, directory is current directory
        directory[0] = '\0';
    }
}

// Helper function to check if path ends with slash (indicating it's a directory)
static bool path_is_directory(char *path) {
    uint32_t len = strlen(path);
    return len > 0 && path[len - 1] == '/';
}

/**
 * @brief Copy a file (folder support is bonus)
 * Supports:
 * - cp file.txt newfile.txt (rename while copying)
 * - cp file.txt dir/ (copy to directory with same name)
 * - cp file.txt /path/to/dir/ (copy to absolute path directory)
 * - cp file.txt /path/to/dir/newname.txt (copy to absolute path with new name)
 * 
 * @param source_path Path to source file/folder
 * @param dest_path Path to destination
 * @param current_inode Current working directory inode
 * @return 0 on success, error code otherwise
 *         1 if source not found
 *         2 if destination already exists
 *         3 if invalid parent directory
 */
int8_t cp_command(char *source_path, char *dest_path, uint32_t current_inode) {
    // First, resolve source path to get parent inode and filename
    uint32_t src_parent_inode = current_inode;
    char src_filename[256];
    
    // Check if source has directory path
    char src_dir[512];
    extract_directory(source_path, src_dir);
    extract_filename(source_path, src_filename);
    
    if (strlen(src_dir) > 0) {
        src_parent_inode = resolve_path(src_dir, current_inode);
        if (src_parent_inode == 0) {
            return 3;  // Invalid source directory
        }
    }
    
    // Now read the source file with correct parent inode
    uint8_t src_buf[CP_BUFFER_SIZE];
    struct EXT2DriverRequest src_request = {
        .buf = src_buf,
        .name = src_filename,
        .name_len = strlen(src_filename),
        .parent_inode = src_parent_inode,
        .buffer_size = CP_BUFFER_SIZE,
        .is_directory = false
    };
    
    int32_t retcode = 0;
    syscall(SYS_READ, (uint32_t)&src_request, (uint32_t)&retcode, 0);
    
    if (retcode != 0) {
        return retcode;
    }
    
    uint32_t src_size = src_request.buffer_size;
    
    // Determine the actual destination path
    uint32_t dest_parent_inode = current_inode;
    char dest_filename[256];
    
    bool dest_is_dir = path_is_directory(dest_path);
    
    // Auto-detect if destination is a directory (even without trailing slash)
    if (!dest_is_dir) {
        // Check if dest_path exists and is a directory
        uint32_t test_dir_inode = resolve_path(dest_path, current_inode);
        if (test_dir_inode != 0) {
            // Path exists, now check if it's a directory by trying to read it
            uint8_t test_buf[BLOCK_SIZE * 2];
            struct EXT2DriverRequest test_request = {
                .buf = test_buf,
                .parent_inode = test_dir_inode,
                .buffer_size = BLOCK_SIZE * 2,
                .is_directory = true
            };
            
            int32_t test_retcode = 0;
            syscall(SYS_READ_DIR, (uint32_t)&test_request, (uint32_t)&test_retcode, 0);
            
            if (test_retcode == 0) {
                // It's a directory, treat as such
                dest_is_dir = true;
            }
        }
    }
    
    if (dest_is_dir) {
        // Destination is a directory - use source filename in that directory
        uint32_t dir_inode = resolve_path(dest_path, current_inode);
        if (dir_inode == 0) {
            return 3;  // Invalid parent directory
        }
        dest_parent_inode = dir_inode;
        memcpy(dest_filename, src_filename, strlen(src_filename));
        dest_filename[strlen(src_filename)] = '\0';
    } else {
        // Destination is a filename (with possible path)
        char dest_dir[512];
        extract_directory(dest_path, dest_dir);
        extract_filename(dest_path, dest_filename);
        
        if (strlen(dest_dir) > 0) {
            dest_parent_inode = resolve_path(dest_dir, current_inode);
            if (dest_parent_inode == 0) {
                return 3;  // Invalid parent directory
            }
        }
    }
    
    // Write to destination
    struct EXT2DriverRequest dst_request = {
        .buf = src_buf,
        .name = dest_filename,
        .name_len = strlen(dest_filename),
        .parent_inode = dest_parent_inode,
        .buffer_size = src_size,
        .is_directory = false
    };
    
    syscall(SYS_WRITE, (uint32_t)&dst_request, (uint32_t)&retcode, 0);
    
    return retcode;
}
