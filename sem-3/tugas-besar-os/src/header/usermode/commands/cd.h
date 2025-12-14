#ifndef _CD_H
#define _CD_H

#include <stdint.h>

/**
 * @brief Change current working directory
 * Supports absolute paths and .. for parent directory
 */
int8_t cd_command(char *path, uint32_t *current_inode);

#endif
