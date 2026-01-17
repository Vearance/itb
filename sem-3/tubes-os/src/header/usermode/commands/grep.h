#ifndef _GREP_H
#define _GREP_H

#include <stdint.h>

/**
 * @brief Search for pattern in file
 */
int8_t grep_command(char *pattern, char *file_path, uint32_t current_inode);

#endif
