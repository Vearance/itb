#include "header/usermode/commands/cat.h"
#include "header/driver/ext2.h"
#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/usermode/user-shell.h"

#define CAT_BUFFER_SIZE 4096

/**
 * @brief Display file contents as text to screen
 * Uses LF newline format
 * 
 * @param file_path Path to the file to display
 * @param current_inode Current working directory inode
 * @return 0 on success, error code otherwise
 *         1 if not a file
 *         2 if file not found
 *         3 if invalid parent directory
 */
int8_t cat_command(char *file_path, uint32_t current_inode) {
    uint8_t buf[BLOCK_SIZE * 8];
    struct EXT2DriverRequest request = {
        .buf = buf,
        .name = file_path,
        .name_len = strlen(file_path),
        .parent_inode = current_inode,
        .buffer_size = BLOCK_SIZE * 8,
        .is_directory = false
    };
    
    int32_t retcode = 0;
    syscall(SYS_READ, (uint32_t)&request, (uint32_t)&retcode, 0);
    
    if (retcode == 0) {
        char *content = (char *)buf;
        uint32_t has_content = 0;
        
        // Print content until null terminator
        for (uint32_t i = 0; content[i] != '\0' && i < BLOCK_SIZE * 8; i++) {
            char c_buf[2] = {content[i], '\0'};
            print_string(c_buf);
            has_content = 1;
        }
        
        // Add newline only if there was content
        if (has_content) {
            print_string("\n");
        }
    }
    
    return retcode;
}
