#ifndef _CP_H
#define _CP_H

#include <stdint.h>

/**
 * @brief Copy a file
 */
int8_t cp_command(char *source_path, char *dest_path, uint32_t current_inode);

#endif
