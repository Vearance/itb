#ifndef _MKDIR_H
#define _MKDIR_H

#include <stdint.h>

/**
 * @brief Create a new empty folder
 */
int8_t mkdir_command(char *folder_name, uint32_t current_inode);

#endif
