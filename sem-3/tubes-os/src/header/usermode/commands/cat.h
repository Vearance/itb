#ifndef _CAT_H
#define _CAT_H

#include <stdint.h>

/**
 * @brief Display file contents as text
 */
int8_t cat_command(char *file_path, uint32_t current_inode);

#endif
