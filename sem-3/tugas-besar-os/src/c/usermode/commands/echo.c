#include "header/usermode/commands/echo.h"
#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/usermode/user-shell.h"
#include "header/driver/ext2.h"

/**
 * @brief Echo command - print string to screen
 * 
 * @param text Text to print
 * @return 0 on success
 */
int8_t echo_command(char *text) {
    if (!text || strlen(text) == 0) {
        print_string("\n");
        return 0;
    }
    
    // Process text and remove quotes
    char output[256] = {0};
    int out_idx = 0;
    int in_quotes = 0;
    
    for (int i = 0; text[i] != '\0' && out_idx < 255; i++) {
        if (text[i] == '"') {
            in_quotes = !in_quotes;  // Toggle quote state
        } else {
            output[out_idx++] = text[i];
        }
    }
    output[out_idx] = '\0';
    
    print_string(output);
    print_string("\n");
    
    return 0;
}

/**
 * @brief Echo with redirect - write string to file
 * 
 * @param text Text to write
 * @param filename Destination file
 * @param current_inode Current working directory inode
 * @return 0 on success, error code otherwise
 */
int8_t echo_redirect_command(char *text, char *filename, uint32_t current_inode) {
    if (!text || !filename || strlen(filename) == 0) {
        return -1;
    }
    
    // Process text and remove quotes
    char output[BLOCK_SIZE * 2];
    memset(output, 0, BLOCK_SIZE * 2);  // Clear entire buffer
    int out_idx = 0;
    int in_quotes = 0;
    
    for (int i = 0; text[i] != '\0' && out_idx < BLOCK_SIZE * 2 - 1; i++) {
        if (text[i] == '"') {
            in_quotes = !in_quotes;  // Toggle quote state
        } else {
            output[out_idx++] = text[i];
        }
    }
    output[out_idx] = '\0';
    
    // Calculate size of content
    uint32_t content_size = out_idx;  // Use actual written size, not strlen
    
    // Try to delete existing file first (if it exists)
    struct EXT2DriverRequest del_request = {
        .buf = NULL,
        .name = filename,
        .name_len = strlen(filename),
        .parent_inode = current_inode,
        .buffer_size = 0,
        .is_directory = false
    };
    
    int32_t del_retcode = 0;
    syscall(SYS_DELETE, (uint32_t)&del_request, (uint32_t)&del_retcode, 0);
    // Ignore error from delete (file might not exist)
    
    // Create or overwrite file with content
    struct EXT2DriverRequest request = {
        .buf = (uint8_t *)output,
        .name = filename,
        .name_len = strlen(filename),
        .parent_inode = current_inode,
        .buffer_size = content_size,
        .is_directory = false
    };
    
    int32_t retcode = 0;
    syscall(SYS_WRITE, (uint32_t)&request, (uint32_t)&retcode, 0);
    
    return retcode;
}

/**
 * @brief Echo with both print and write - for pipe support
 * Prints to screen AND writes to file
 * 
 * @param text Text to output
 * @param filename Destination file
 * @param current_inode Current working directory inode
 * @return 0 on success, error code otherwise
 */
int8_t echo_pipe_command(char *text, char *filename, uint32_t current_inode) {
    if (!text || !filename || strlen(filename) == 0) {
        return -1;
    }
    
    // Process text and remove quotes
    char output[BLOCK_SIZE * 2];
    memset(output, 0, BLOCK_SIZE * 2);
    int out_idx = 0;
    int in_quotes = 0;
    
    for (int i = 0; text[i] != '\0' && out_idx < BLOCK_SIZE * 2 - 1; i++) {
        if (text[i] == '"') {
            in_quotes = !in_quotes;
        } else {
            output[out_idx++] = text[i];
        }
    }
    output[out_idx] = '\0';
    
    // Write to file (do NOT print to screen in pipe mode)
    uint32_t content_size = out_idx;
    
    // Try to delete existing file first (if it exists)
    struct EXT2DriverRequest del_request = {
        .buf = NULL,
        .name = filename,
        .name_len = strlen(filename),
        .parent_inode = current_inode,
        .buffer_size = 0,
        .is_directory = false
    };
    
    int32_t del_retcode = 0;
    syscall(SYS_DELETE, (uint32_t)&del_request, (uint32_t)&del_retcode, 0);
    // Ignore error from delete (file might not exist)
    
    struct EXT2DriverRequest request = {
        .buf = (uint8_t *)output,
        .name = filename,
        .name_len = strlen(filename),
        .parent_inode = current_inode,
        .buffer_size = content_size,
        .is_directory = false
    };
    
    int32_t retcode = 0;
    syscall(SYS_WRITE, (uint32_t)&request, (uint32_t)&retcode, 0);
    
    return retcode;
}