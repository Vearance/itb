#ifndef _USER_SHELL_H
#define _USER_SHELL_H

#include <stdint.h>
#include <stdbool.h>
#include "header/driver/ext2.h"
#include "header/text/framebuffer.h"
#include "header/interrupt/interrupt.h"
#include "header/usermode/commands/touch.h"
#include "header/usermode/commands/echo.h"
#include "header/graphics/graphics.h"
#include "header/graphics/font.h"

// Additional color definitions (graphics.h already defines COLOR_*)
#define COLOR_GRAY          7
#define COLOR_LIGHT_MAGENTA 13  // Light Pink / Pink

// Background color (shift by 8)
#define BG_BLACK            (COLOR_BLACK << 8)
#define BG_BLUE             (COLOR_BLUE << 8)

#define COLOR_FOLDER        COLOR_CYAN
#define COLOR_FOUND_FILE    COLOR_LIGHT_MAGENTA  // Light Pink
#define COLOR_DEFAULT       COLOR_GRAY
#define BG_DEFAULT          BG_BLACK

#define BG_ENV_MAX 256

// Init script filename
#define INIT_FILE_NAME "hutaos.init"

// Directory information structure
typedef struct {
    char dir_name[12];
    uint8_t dir_name_len;
    uint32_t inode;
} dir_info;

// Absolute directory information structure
typedef struct {
    int current_dir;
    dir_info dir[50];
} absolute_dir_info;

// Shell state structure
typedef struct {
    char input_buffer[256];
    int input_length;
    int cursor_position;
} ShellState;

// External variables
extern absolute_dir_info DIR_INFO;
extern ShellState shell_state;
extern char current_dir_name[256];
extern uint32_t current_dir_inode;
extern char bg_env[BG_ENV_MAX];

/**
 * @brief Print a string to framebuffer with cursor tracking
 * @param str String to print
 */
void print_string(const char *str);

/**
 * @brief Print a string with custom foreground color
 * @param str String to print
 * @param color Foreground color
 */
void print_string_color(const char *str, uint8_t color);

/**
 * @brief Print a string with custom foreground color and specific length
 * @param str String to print
 * @param len Length of string to print
 * @param color Foreground color
 */
void print_string_color_len(const char *str, uint32_t len, uint8_t color);

/**
 * @brief Print error message
 * @param msg Error message to print
 */
void print_error(const char *msg);

/**
 * @brief Print current working directory prompt
 * @param current_inode Current working directory inode
 */
void print_cwd(uint32_t current_inode);

/**
 * @brief Read a line from keyboard input
 * @param buffer Buffer to store input
 * @param max_len Maximum length to read
 * @return Number of characters read
 */
uint32_t read_line(char *buffer, uint32_t max_len);

/**
 * @brief Execute a command
 * @param cmd_line Command line to execute
 * @param current_inode Pointer to current inode
 * @return 0 on success, error code otherwise
 */
int8_t execute_command(char *cmd_line, uint32_t *current_inode);

/**
 * @brief Get current cursor row
 * @return Current cursor row
 */
uint32_t get_cursor_row(void);

/**
 * @brief Get current cursor column
 * @return Current cursor column
 */
uint32_t get_cursor_col(void);

/**
 * @brief Set cursor position
 * @param row Row position
 * @param col Column position
 */
void set_cursor_pos(uint32_t row, uint32_t col);

/**
 * @brief Make a syscall to kernel
 * @param eax Syscall number
 * @param ebx First parameter
 * @param ecx Second parameter
 * @param edx Third parameter
 */
void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

// Custom font utilities (using font.h bitmap data)
// Font is 5 pixels wide x 8 pixels tall (matching graphics_reference.c)
#define CUSTOM_FONT_WIDTH 5
#define CUSTOM_FONT_HEIGHT 8

#endif
