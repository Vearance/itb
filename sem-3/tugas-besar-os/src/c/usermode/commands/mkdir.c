#include "header/usermode/commands/mkdir.h"
#include "header/driver/ext2.h"
#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/usermode/user-shell.h"

/**
 * @brief Create a new empty folder in current working directory
 * 
 * @param folder_name Name of the folder to create
 * @param current_inode Current working directory inode
 * @return 0 on success, error code otherwise
 *         1 if folder already exists
 *         2 if invalid parent directory
 */
int8_t mkdir_command(char *folder_name, uint32_t current_inode) {
    struct EXT2DriverRequest request = {
        .buf = NULL,
        .name = folder_name,
        .name_len = strlen(folder_name),
        .parent_inode = current_inode,
        .buffer_size = 0,
        .is_directory = true
    };
    
    int32_t retcode = 0;
    syscall(SYS_WRITE, (uint32_t)&request, (uint32_t)&retcode, 0);
    
    return retcode;
}
