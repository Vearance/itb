#ifndef _FIND_H
#define _FIND_H

#include <stdint.h>

/**
 * @brief Find file/folder by name in specified path
 * 
 * @param search_path Path to search in ("." for current, "/" for root, "dirname" for subdirectory)
 * @param filename Name to search for
 * @param current_inode Current working directory inode
 */
int8_t find_command(char *search_path, char *filename, uint32_t current_inode);

#endif
