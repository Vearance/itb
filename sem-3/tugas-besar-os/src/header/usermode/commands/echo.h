#ifndef _ECHO_H
#define _ECHO_H

#include <stdint.h>

/**
 * @brief Echo command - print string to screen
 * 
 * @param text Text to print
 * @return 0 on success
 */
int8_t echo_command(char *text);

/**
 * @brief Echo with redirect - write string to file
 * 
 * @param text Text to write
 * @param filename Destination file
 * @param current_inode Current working directory inode
 * @return 0 on success, error code otherwise
 */
int8_t echo_redirect_command(char *text, char *filename, uint32_t current_inode);

/**
 * @brief Echo with pipe support - write to file (no screen output in pipe mode)
 * Used when echo is part of a pipe chain
 * 
 * @param text Text to write
 * @param filename Destination file
 * @param current_inode Current working directory inode
 * @return 0 on success, error code otherwise
 */
int8_t echo_pipe_command(char *text, char *filename, uint32_t current_inode);

#endif
