#ifndef _LS_H
#define _LS_H

#include <stdint.h>

/**
 * @brief List contents of directory
 */
int8_t ls_command(char *path, uint32_t current_inode);

#endif
