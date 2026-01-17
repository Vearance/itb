#include "header/usermode/commands/touch.h"
#include "header/driver/ext2.h"
#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/usermode/user-shell.h"

/**
 * @brief Create a new empty file in current working directory
 * 
 * @param file_name Name of the file to create
 * @param current_inode Current working directory inode
 * @return 0 on success, error code otherwise
 *         1 if file already exists
 *         2 if invalid parent directory
 */
int8_t touch_command(char *file_name, uint32_t current_inode) {
    if (!file_name || strlen(file_name) == 0) {
        return -1;
    }
    
    struct EXT2DriverRequest request = {
        .buf = NULL,
        .name = file_name,
        .name_len = strlen(file_name),
        .parent_inode = current_inode,
        .buffer_size = 0,
        .is_directory = false
    };
    
    int32_t retcode = 0;
    syscall(SYS_WRITE, (uint32_t)&request, (uint32_t)&retcode, 0);
    
    return retcode;
}
