#ifndef _MV_H
#define _MV_H

#include <stdint.h>

/**
 * @brief Move and rename file/folder
 */
int8_t mv_command(char *source_path, char *dest_path, uint32_t current_inode);

#endif
