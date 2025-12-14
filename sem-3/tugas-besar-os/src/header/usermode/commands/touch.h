#ifndef _TOUCH_H
#define _TOUCH_H

#include <stdint.h>

/**
 * @brief Create a new empty file in current working directory
 * 
 * @param file_name Name of the file to create
 * @param current_inode Current working directory inode
 * @return 0 on success, error code otherwise
 *         1 if file already exists
 *         2 if invalid parent directory
 */
int8_t touch_command(char *file_name, uint32_t current_inode);

#endif
