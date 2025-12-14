#include "header/usermode/commands/grep.h"
#include "header/driver/ext2.h"
#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/usermode/user-shell.h"

#define GREP_BUFFER_SIZE (BLOCK_SIZE * 8)

/**
 * @brief Search for pattern in file
 * Prints matching lines
 * 
 * @param pattern Pattern to search for
 * @param file_path Path to file to search in
 * @param current_inode Current working directory inode
 * @return 0 on success (or partial match), error code otherwise
 *         1 if file not found
 */
int8_t grep_command(char *pattern, char *file_path, uint32_t current_inode) {
    if (!pattern || !file_path || strlen(pattern) == 0 || strlen(file_path) == 0) {
        return -1;
    }
    
    uint8_t buf[GREP_BUFFER_SIZE];
    struct EXT2DriverRequest request = {
        .buf = buf,
        .name = file_path,
        .name_len = strlen(file_path),
        .parent_inode = current_inode,
        .buffer_size = GREP_BUFFER_SIZE,
        .is_directory = false
    };
    
    int32_t retcode = 0;
    syscall(SYS_READ, (uint32_t)&request, (uint32_t)&retcode, 0);
    
    if (retcode != 0) {
        return retcode;
    }
    
    char *content = (char *)buf;
    // Use strlen to get actual content length, buffer is null-terminated by read function
    uint32_t size = strlen(content);
    uint32_t pattern_len = strlen(pattern);
    
    // Search for pattern line by line
    uint32_t line_start = 0;
    
    for (uint32_t i = 0; i <= size; i++) {
        if (i == size || content[i] == '\n') {
            uint32_t line_len = i - line_start;
            
            // Search for pattern in this line
            for (uint32_t j = 0; j + pattern_len <= line_len; j++) {
                if (memcmp(&content[line_start + j], pattern, pattern_len) == 0) {
                    // Print matching line with pattern highlighted in light magenta
                    uint32_t k = 0;
                    while (k < line_len) {
                        // Check if pattern starts at this position
                        if (k + pattern_len <= line_len && 
                            memcmp(&content[line_start + k], pattern, pattern_len) == 0) {
                            // Print pattern in light red (color 12)
                            print_string_color_len(&content[line_start + k], pattern_len, 12);
                            k += pattern_len;
                        } else {
                            // Print normal character
                            char c = content[line_start + k];
                            print_string(&c);
                            k++;
                        }
                    }
                    print_string("\n");
                    break;
                }
            }
            
            line_start = i + 1;
        }
    }
    
    return 0;
}
