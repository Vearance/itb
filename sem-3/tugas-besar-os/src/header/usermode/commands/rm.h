#ifndef _RM_H
#define _RM_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Remove a file or directory
 * @param target_path Path to file/directory to remove
 * @param current_inode Current working directory inode
 * @param recursive If true, recursively delete directories and contents. If false, only delete empty directories/files
 * @return 0 on success, error code otherwise
 *         1 if not found
 *         2 if folder is not empty (and not recursive)
 *         3 if invalid parent directory
 */
int8_t rm_command(char *target_path, uint32_t current_inode, bool recursive);

#endif
