#include <stdint.h>
#include "header/driver/ext2.h"
#include "header/text/framebuffer.h"
#include "header/graphics/graphics.h"
#include "header/graphics/font.h"
#include "header/stdlib/string.h"
#include "header/interrupt/interrupt.h"
#include "header/usermode/user-shell.h"
#include "header/usermode/user-heap.h"
#include "header/usermode/commands/cd.h"
#include "header/usermode/commands/ls.h"
#include "header/usermode/commands/mkdir.h"
#include "header/usermode/commands/cat.h"
#include "header/usermode/commands/cp.h"
#include "header/usermode/commands/rm.h"
#include "header/usermode/commands/mv.h"
#include "header/usermode/commands/find.h"
#include "header/usermode/commands/grep.h"
#include "header/usermode/commands/touch.h"
#include "header/usermode/commands/echo.h"
#include "header/usermode/commands/exec.h"
#include "header/usermode/commands/ps.h"
#include "header/usermode/commands/kill.h"
#include "header/usermode/commands/bg.h"

#define BLOCK_COUNT 16
#define INPUT_BUFFER_SIZE 256

// Arrow key codes (must match keyboard.h)
#define ARROW_UP    0x10
#define ARROW_LEFT  0x11
#define ARROW_RIGHT 0x12
#define ARROW_DOWN  0x13

// Control key codes (must match keyboard.h)
#define CTRL_C      0x03  // Interrupt signal

// Command history settings
#define HISTORY_SIZE 10
#define HISTORY_CMD_LEN 256

static char command_history[HISTORY_SIZE][HISTORY_CMD_LEN];
static int history_count = 0;      // Total commands in history

// Environment persistence
#define INIT_FILE_NAME "hutaos.init"

#define MAX_PATH_DIRS 16
#define MAX_PATH_LEN 256

static char path_dirs[MAX_PATH_DIRS][MAX_PATH_LEN];
static uint32_t path_count = 0;

// Graphics mode settings (320x200)
#define GFX_WIDTH 320
#define GFX_HEIGHT 200
#define GFX_CHAR_WIDTH 5   // 5 pixels wide (matching graphics_reference.c)
#define GFX_CHAR_HEIGHT 8
#define GFX_COLS (GFX_WIDTH / GFX_CHAR_WIDTH)   // 64 columns
#define GFX_ROWS ((GFX_HEIGHT - GFX_CHAR_HEIGHT) / GFX_CHAR_HEIGHT) // 24 rows (reserve last row for clock)

// Special color for transparent background (uses wallpaper)
#define COLOR_TRANSPARENT 0xFF

// Screen text buffer for scroll support
// Stores character and color at each position
typedef struct {
    char chars[GFX_ROWS][GFX_COLS];
    uint8_t colors[GFX_ROWS][GFX_COLS];
} ScreenBuffer;

static ScreenBuffer screen_buffer = {0};

// Initialize screen buffer with spaces
__attribute__((unused)) static void screen_buffer_init(void) {
    for (uint32_t row = 0; row < GFX_ROWS; row++) {
        for (uint32_t col = 0; col < GFX_COLS; col++) {
            screen_buffer.chars[row][col] = ' ';
            screen_buffer.colors[row][col] = COLOR_WHITE;
        }
    }
}

// Set character in screen buffer
static void screen_buffer_set(uint32_t row, uint32_t col, char c, uint8_t color) {
    if (row < GFX_ROWS && col < GFX_COLS) {
        screen_buffer.chars[row][col] = c;
        screen_buffer.colors[row][col] = color;
    }
}

// Clear a row in screen buffer
static void screen_buffer_clear_row(uint32_t row) {
    if (row < GFX_ROWS) {
        for (uint32_t col = 0; col < GFX_COLS; col++) {
            screen_buffer.chars[row][col] = ' ';
            screen_buffer.colors[row][col] = COLOR_WHITE;
        }
    }
}

// Clear entire screen buffer
static void screen_buffer_clear(void) {
    for (uint32_t row = 0; row < GFX_ROWS; row++) {
        screen_buffer_clear_row(row);
    }
}

// Legacy text mode settings (for compatibility)
#define FRAMEBUFFER_WIDTH 80
#define FRAMEBUFFER_HEIGHT 25
#define OUTPUT_BUFFER_SIZE 4096

// Graphics mode flag
static bool use_graphics_mode = true;

/**
 * Graphics mode syscall helpers
 */
static void gfx_init(void)
{
    syscall(SYS_GFX_INIT, 0, 0, 0);
}

static void gfx_putchar(uint16_t x, uint16_t y, char c, uint8_t fg, uint8_t bg)
{
    uint32_t pos = (uint32_t)x | ((uint32_t)y << 16);
    uint32_t char_info = (uint32_t)c | ((uint32_t)fg << 8) | ((uint32_t)bg << 16);
    syscall(SYS_GFX_PUTCHAR, pos, char_info, 0);
}

__attribute__((unused)) static void gfx_puts(uint16_t x, uint16_t y, const char *str, uint8_t fg, uint8_t bg)
{
    uint32_t pos = (uint32_t)x | ((uint32_t)y << 16);
    uint32_t colors = (uint32_t)fg | ((uint32_t)bg << 8);
    syscall(SYS_GFX_PUTS, pos, (uint32_t)str, colors);
}

__attribute__((unused)) static void gfx_clear(uint8_t color)
{
    syscall(SYS_GFX_CLEAR, color, 0, 0);
}

__attribute__((unused)) static void gfx_scroll(uint16_t lines, uint8_t color)
{
    syscall(SYS_GFX_SCROLL, lines, color, 0);
}

static void gfx_draw_wallpaper(void)
{
    syscall(SYS_GFX_DRAW_WALLPAPER, 0, 0, 0);
}

static void gfx_restore_bg(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    uint32_t pos = (uint32_t)x | ((uint32_t)y << 16);
    uint32_t size = (uint32_t)width | ((uint32_t)height << 16);
    syscall(SYS_GFX_RESTORE_BG, pos, size, 0);
}

static void gfx_draw_cursor(uint16_t x, uint16_t y, uint8_t color)
{
    // Draw a vertical line cursor (|) - 1 pixel wide, 8 pixels tall
    for (uint16_t row = 0; row < 8; row++)
    {
        uint32_t pos = (uint32_t)x | ((uint32_t)(y + row) << 16);
        syscall(SYS_GFX_PIXEL, pos, color, 0);
    }
}

static void gfx_erase_cursor(uint16_t x, uint16_t y)
{
    // Erase cursor by restoring wallpaper background
    // Restore 5px width to ensuring full cell is cleared
    gfx_restore_bg(x, y, GFX_CHAR_WIDTH, GFX_CHAR_HEIGHT);

    // Redraw character from buffer if it exists
    uint32_t col = x / GFX_CHAR_WIDTH;
    uint32_t row = y / GFX_CHAR_HEIGHT;

    if (row < GFX_ROWS && col < GFX_COLS) {
        char c = screen_buffer.chars[row][col];
        // If there is a printable character, redraw it
        if (c != ' ' && c != '\0') {
            gfx_putchar(x, y, c, screen_buffer.colors[row][col], COLOR_TRANSPARENT);
        }
    }
}

// Scroll the screen buffer up by one row and redraw
static void screen_buffer_scroll(void) {
    // Shift all rows up by 1
    for (uint32_t row = 0; row < GFX_ROWS - 1; row++) {
        for (uint32_t col = 0; col < GFX_COLS; col++) {
            screen_buffer.chars[row][col] = screen_buffer.chars[row + 1][col];
            screen_buffer.colors[row][col] = screen_buffer.colors[row + 1][col];
        }
    }
    
    // Clear the last row
    screen_buffer_clear_row(GFX_ROWS - 1);
    
    // Redraw entire screen: first restore wallpaper, then draw all text
    gfx_draw_wallpaper();
    
    // Redraw all characters from buffer
    for (uint32_t row = 0; row < GFX_ROWS; row++) {
        for (uint32_t col = 0; col < GFX_COLS; col++) {
            char c = screen_buffer.chars[row][col];
            if (c != ' ' && c != '\0') {
                uint16_t x = col * GFX_CHAR_WIDTH;
                uint16_t y = row * GFX_CHAR_HEIGHT;
                gfx_putchar(x, y, c, screen_buffer.colors[row][col], COLOR_TRANSPARENT);
            }
        }
    }
}

// Public function to refresh screen (redraw wallpaper and text buffer)
// Can be called from other modules like kill.c
void refresh_screen(void) {
    gfx_draw_wallpaper();
    
    // Redraw all characters from buffer
    for (uint32_t row = 0; row < GFX_ROWS; row++) {
        for (uint32_t col = 0; col < GFX_COLS; col++) {
            char c = screen_buffer.chars[row][col];
            if (c != ' ' && c != '\0') {
                uint16_t x = col * GFX_CHAR_WIDTH;
                uint16_t y = row * GFX_CHAR_HEIGHT;
                gfx_putchar(x, y, c, screen_buffer.colors[row][col], COLOR_TRANSPARENT);
            }
        }
    }
}

/**
 * Get custom font data for a character
 * Returns pointer to font bitmap data from font.h
 */
__attribute__((unused)) static const uint8_t* get_custom_font_char(unsigned char c)
{
    if (c >= 128) {
        return lookup[0]; // Return empty char for unsupported
    }
    return lookup[c];
}

/**
 * Get number of pixels lit in custom font character
 */
__attribute__((unused)) static uint8_t get_custom_font_char_pixel_count(unsigned char c)
{
    const uint8_t* char_data = get_custom_font_char(c);
    return char_data[0]; // First byte is pixel count
}

/**
 * Check if a specific pixel is lit in custom font character
 * cx: column (0-7), cy: row (0-7)
 */
__attribute__((unused)) static bool is_custom_font_pixel_lit(unsigned char c, uint8_t cx, uint8_t cy)
{
    if (c >= 128 || cx >= 8 || cy >= 8) {
        return false;
    }

    const uint8_t* char_data = get_custom_font_char(c);
    uint8_t num_pixels = char_data[0];
    
    // Font format: high nibble = col, low nibble = row
    uint8_t pixel_pos = (cx << 4) | cy;
    
    for (uint8_t i = 1; i <= num_pixels; i++) {
        if (char_data[i] == pixel_pos) {
            return true;
        }
    }
    
    return false;
}

// Clock area management - not used in graphics mode
__attribute__((unused)) static bool clock_hidden = false;

// Helper to hide clock area - not used in graphics mode (clock has its own area)
__attribute__((unused)) static void hide_clock_area(void)
{
    clock_hidden = true;
}

// Helper to show clock area again - not used in graphics mode
__attribute__((unused)) static void show_clock_area(void)
{
    clock_hidden = false;
}

// Output buffer structure
typedef struct
{
    char buffer[OUTPUT_BUFFER_SIZE];
    uint8_t colors[OUTPUT_BUFFER_SIZE]; // Color for each character
    uint32_t position;                  // Current write position
} OutputBuffer;

// Cursor tracking
static uint32_t cursor_row = 0;
static uint32_t cursor_col = 0;

// Output buffer instance
static OutputBuffer output_buffer = {0};

// Global variables
absolute_dir_info DIR_INFO = {0};
ShellState shell_state = {0};
char current_dir_name[256] = {0};
uint32_t current_dir_inode = 1;
char current_path[512] = {0}; // Full path from root
uint32_t path_depth = 0;      // Track depth in directory hierarchy
char bg_env[BG_ENV_MAX] = "/bg";

/**
 * Parse command line to extract parts separated by pipe '|' operator
 * Returns: number of commands (1 if no pipe, >1 if pipe found)
 * commands array contains each command
 */
uint32_t parse_pipe(const char *input, char (*commands)[512], uint32_t max_commands)
{
    uint32_t cmd_count = 0;
    uint32_t i = 0;
    uint32_t j = 0;
    int in_quote = 0;
    uint32_t input_len = strlen(input);

    while (i < input_len && cmd_count < max_commands)
    {
        // Track quotes to avoid counting | inside quotes
        if (input[i] == '"')
        {
            in_quote = !in_quote;
            commands[cmd_count][j++] = input[i++];
        }
        else if (input[i] == '|' && !in_quote)
        {
            // Found pipe operator
            commands[cmd_count][j] = '\0';
            cmd_count++;
            i++; // Skip the |

            // Skip spaces after |
            while (i < input_len && input[i] == ' ')
            {
                i++;
            }

            // Start collecting next command
            j = 0;
        }
        else
        {
            commands[cmd_count][j++] = input[i++];
        }
    }

    commands[cmd_count][j] = '\0';
    cmd_count++;

    return cmd_count;
}

/**
 * Parse command line to extract parts separated by redirect '>' operator
 * Returns: number of parts (1 if no redirect, 2 if redirect found)
 * parts[0] = command arguments before redirect
 * parts[1] = filename/path after redirect
 */
uint32_t parse_redirect(const char *input, char (*parts)[512], uint32_t max_parts)
{
    uint32_t part_count = 0;
    uint32_t i = 0;
    uint32_t j = 0;
    int in_quote = 0;
    uint32_t input_len = strlen(input);

    while (i < input_len && part_count < max_parts)
    {
        // Track quotes to avoid counting > inside quotes
        if (input[i] == '"')
        {
            in_quote = !in_quote;
            parts[part_count][j++] = input[i++];
        }
        else if (input[i] == '>' && !in_quote)
        {
            // Found redirect operator
            parts[part_count][j] = '\0';
            part_count++;
            i++; // Skip the >

            // Skip spaces after >
            while (i < input_len && input[i] == ' ')
            {
                i++;
            }

            // Start collecting second part
            j = 0;
        }
        else
        {
            parts[part_count][j++] = input[i++];
        }
    }

    parts[part_count][j] = '\0';
    part_count++;

    return part_count;
}

/**
 * Create directory path and return the inode of the final directory
 * If path contains multiple levels, create all intermediate directories
 * path should be relative to current_inode
 */
uint32_t create_path_and_return_inode(const char *path, uint32_t current_inode)
{
    if (!path || strlen(path) == 0)
    {
        return current_inode;
    }

    // Make a copy since we'll be modifying it
    char path_copy[512] = {0};
    memcpy(path_copy, path, strlen(path));

    uint32_t current = current_inode;
    uint32_t i = 0;
    uint32_t path_len = strlen(path_copy);

    while (i < path_len)
    {
        // Skip leading slashes
        while (i < path_len && path_copy[i] == '/')
        {
            i++;
        }

        if (i >= path_len)
            break;

        // Extract next path component
        char token[256] = {0};
        uint32_t j = 0;
        while (i < path_len && path_copy[i] != '/' && j < 255)
        {
            token[j++] = path_copy[i++];
        }
        token[j] = '\0';

        if (strlen(token) > 0)
        {
            // Try to cd into this directory, if it doesn't exist, create it
            uint8_t temp_buf[BLOCK_SIZE];
            struct EXT2DriverRequest check_req = {
                .buf = temp_buf,
                .name = token,
                .name_len = strlen(token),
                .parent_inode = current,
                .buffer_size = BLOCK_SIZE,
                .is_directory = true};

            int32_t check_retcode = 0;
            syscall(SYS_READ_DIR, (uint32_t)&check_req, (uint32_t)&check_retcode, 0);

            if (check_retcode == 0)
            {
                // Directory exists, get its inode
                struct EXT2DirectoryEntry *entry = (struct EXT2DirectoryEntry *)temp_buf;
                current = entry->inode;
            }
            else
            {
                // Directory doesn't exist, create it
                struct EXT2DriverRequest mkdir_req = {
                    .buf = NULL,
                    .name = token,
                    .name_len = strlen(token),
                    .parent_inode = current,
                    .buffer_size = 0,
                    .is_directory = true};

                int32_t mkdir_retcode = 0;
                syscall(SYS_WRITE, (uint32_t)&mkdir_req, (uint32_t)&mkdir_retcode, 0);

                if (mkdir_retcode == 0)
                {
                    // Directory created, read it to get its inode
                    syscall(SYS_READ_DIR, (uint32_t)&check_req, (uint32_t)&check_retcode, 0);
                    if (check_retcode == 0)
                    {
                        struct EXT2DirectoryEntry *entry = (struct EXT2DirectoryEntry *)temp_buf;
                        current = entry->inode;
                    }
                }
                else
                {
                    // Failed to create directory
                    return 0;
                }
            }
        }
    }

    return current;
}

/**
 * Flush output buffer to framebuffer
 * Writes all buffered characters to the screen with cursor management
 */
void flush_output_buffer(void)
{
    uint32_t max_cols = use_graphics_mode ? GFX_COLS : FRAMEBUFFER_WIDTH;
    uint32_t max_rows = use_graphics_mode ? GFX_ROWS : FRAMEBUFFER_HEIGHT;

    for (uint32_t i = 0; i < output_buffer.position; i++)
    {
        char c = output_buffer.buffer[i];
        uint8_t color = output_buffer.colors[i];

        // Only print valid printable characters and newline
        if ((c >= 32 && c <= 126) || c == '\n')
        {
            if (use_graphics_mode)
            {
                // Graphics mode: use custom font with transparent background
                if (c != '\n')
                {
                    uint16_t x = cursor_col * GFX_CHAR_WIDTH;
                    uint16_t y = cursor_row * GFX_CHAR_HEIGHT;
                    gfx_putchar(x, y, c, color, COLOR_TRANSPARENT);
                    // Store in screen buffer for scroll support
                    screen_buffer_set(cursor_row, cursor_col, c, color);
                }
            }
            else
            {
                // Text mode: use framebuffer
                CursorPosition cp = {cursor_row, cursor_col};
                syscall(SYS_PUTCHAR, (uint32_t)c, color, (uint32_t)&cp);
            }

            cursor_col++;
            if (cursor_col >= max_cols || c == '\n')
            {
                cursor_col = 0;
                cursor_row++;

                // If we've reached the bottom, scroll up
                if (cursor_row >= max_rows)
                {
                    if (use_graphics_mode)
                    {
                        screen_buffer_scroll();
                    }
                    else
                    {
                        syscall(SYS_SCROLL, 0, 0, 0);
                    }
                    cursor_row = max_rows - 1;
                }
            }
        }
    }

    // Update hardware cursor position (only in text mode)
    if (!use_graphics_mode)
    {
        CursorPosition cp = {cursor_row, cursor_col};
        syscall(SYS_SET_CURSOR, (uint32_t)&cp, 0, 0);
    }

    // Reset buffer
    output_buffer.position = 0;
    memset(output_buffer.buffer, 0, OUTPUT_BUFFER_SIZE);
    memset(output_buffer.colors, 0, OUTPUT_BUFFER_SIZE);
}

/**
 * Reset output buffer without flushing
 */
void reset_output_buffer(void)
{
    output_buffer.position = 0;
    memset(output_buffer.buffer, 0, OUTPUT_BUFFER_SIZE);
    memset(output_buffer.colors, 0, OUTPUT_BUFFER_SIZE);
}

// Current color for output
static uint8_t current_color = 0x0F;

/**
 * Set color for subsequent output
 */
void set_output_color(uint8_t color)
{
    current_color = color;
}

/**
 * Add character to output buffer
 * Automatically flushes if buffer is almost full
 */
void buffer_putchar(char c)
{
    if (output_buffer.position >= OUTPUT_BUFFER_SIZE - 1)
    {
        flush_output_buffer();
    }
    output_buffer.buffer[output_buffer.position] = c;
    output_buffer.colors[output_buffer.position] = current_color;
    output_buffer.position++;
}

/**
 * Add string to output buffer
 * Handles flushing if needed
 */
void buffer_putstring(const char *str)
{
    while (*str)
    {
        buffer_putchar(*str);
        str++;
    }
}

/**
 * Add string to output buffer with specific color
 */
void buffer_putstring_color(const char *str, uint8_t color)
{
    uint8_t old_color = current_color;
    set_output_color(color);
    while (*str)
    {
        buffer_putchar(*str);
        str++;
    }
    set_output_color(old_color);
}

/**
 * Add string with length to output buffer
 */
void buffer_putstring_len(const char *str, uint32_t len)
{
    uint32_t i = 0;
    while (i < len && str[i])
    {
        buffer_putchar(str[i]);
        i++;
    }
}

/**
 * Add string with length to output buffer with specific color
 */
void buffer_putstring_len_color(const char *str, uint32_t len, uint8_t color)
{
    uint8_t old_color = current_color;
    set_output_color(color);
    uint32_t i = 0;
    while (i < len && str[i])
    {
        buffer_putchar(str[i]);
        i++;
    }
    set_output_color(old_color);
}

/**
 * Get current buffer fill percentage
 */
uint32_t get_buffer_fill_percent(void)
{
    return (output_buffer.position * 100) / OUTPUT_BUFFER_SIZE;
}

/**
 * Get current cursor row
 */
uint32_t get_cursor_row(void)
{
    return cursor_row;
}

/**
 * Get current cursor column
 */
uint32_t get_cursor_col(void)
{
    return cursor_col;
}

/**
 * Set cursor position
 */
void set_cursor_pos(uint32_t row, uint32_t col)
{
    cursor_row = row;
    cursor_col = col;
}

void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx)
{
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

/**
 * Print a string to output buffer
 */
void print_string(const char *str)
{
    buffer_putstring(str);
}

/**
 * Print string with custom foreground color
 */
void print_string_color(const char *str, uint8_t color)
{
    buffer_putstring_color(str, color);
}

/**
 * Print a string with specific length and color
 */
void print_string_color_len(const char *str, uint32_t len, uint8_t color)
{
    buffer_putstring_len_color(str, len, color);
}

/**
 * Print current working directory
 */
void print_cwd(uint32_t current_inode __attribute__((unused)))
{
    print_string_color("HuTaOS", COLOR_RED);
    print_string_color(":", COLOR_WHITE);
    if (current_dir_inode == 1)
    {
        print_string_color("/", COLOR_CYAN);
    }
    else
    {
        print_string_color("/", COLOR_CYAN);
        if (current_path[0] != '\0')
        {
            print_string_color(current_path, COLOR_CYAN);
        }
    }
    print_string_color("$", COLOR_WHITE);
    print_string(" ");
}

/**
 * Print error message and flush buffer
 */
void print_error(const char *msg)
{
    print_string(msg);
    print_string("\n");
    flush_output_buffer();
}

/**
 * Add command to history
 */
static void history_add(const char *cmd)
{
    uint32_t cmd_len = strlen(cmd);
    if (cmd_len == 0) return;
    
    // Don't add if same as last command
    if (history_count > 0) {
        const char *last_cmd = command_history[(history_count - 1) % HISTORY_SIZE];
        if ((uint32_t)strlen(last_cmd) == cmd_len && memcmp(last_cmd, cmd, cmd_len) == 0)
            return;
    }
    
    // Add to circular buffer
    int idx = history_count % HISTORY_SIZE;
    for (uint32_t i = 0; i < HISTORY_CMD_LEN - 1 && cmd[i]; i++)
        command_history[idx][i] = cmd[i];
    command_history[idx][strlen(cmd)] = '\0';
    
    history_count++;
}

/**
 * Get command from history
 * @param index: 0 = most recent, 1 = second most recent, etc.
 * @return pointer to command or NULL if not found
 */
static const char* history_get(int index)
{
    if (index < 0 || index >= history_count || index >= HISTORY_SIZE)
        return NULL;
    
    int actual_idx = (history_count - 1 - index) % HISTORY_SIZE;
    if (actual_idx < 0) actual_idx += HISTORY_SIZE;
    return command_history[actual_idx];
}

/**
 * Read a line from keyboard input
 */
uint32_t read_line(char *buffer, uint32_t max_len)
{
    uint32_t len = 0;           // Current length of input
    uint32_t cursor_pos = 0;    // Cursor position within input buffer
    char c = 0;
    bool cursor_visible = true;   // Start with cursor visible
    uint8_t last_second = 0xFF;   // Track last second for 1-second blink
    uint32_t max_cols = use_graphics_mode ? GFX_COLS : FRAMEBUFFER_WIDTH;
    uint32_t max_rows = use_graphics_mode ? GFX_ROWS : FRAMEBUFFER_HEIGHT;
    
    // Store starting position for input (after prompt)
    uint32_t start_row = cursor_row;
    uint32_t start_col = cursor_col;
    
    // Temporary buffer for current input before history navigation
    char temp_buffer[HISTORY_CMD_LEN] = {0};
    int current_history_nav = -1;  // -1 = current input, 0+ = history index

    // Get initial time
    struct { uint8_t seconds; uint8_t minutes; uint8_t hours; } init_time;
    if (use_graphics_mode)
    {
        syscall(SYS_TIME_GET, (uint32_t)&init_time, 0, 0);
        last_second = init_time.seconds;
    }

    // Set initial cursor position
    if (!use_graphics_mode)
    {
        CursorPosition cp_init = {cursor_row, cursor_col};
        syscall(SYS_SET_CURSOR, (uint32_t)&cp_init, 0, 0);
    }
    else
    {
        // Draw initial cursor in graphics mode
        uint16_t x = cursor_col * GFX_CHAR_WIDTH;
        uint16_t y = cursor_row * GFX_CHAR_HEIGHT;
        gfx_draw_cursor(x, y, COLOR_WHITE);
        cursor_visible = true;
    }

    while (len < max_len - 1)
    {
        // Handle cursor blinking in graphics mode BEFORE checking for input
        if (use_graphics_mode)
        {
            // Get current time for 1-second blink
            struct { uint8_t seconds; uint8_t minutes; uint8_t hours; } cur_time;
            syscall(SYS_TIME_GET, (uint32_t)&cur_time, 0, 0);
            
            // Toggle cursor every second
            if (cur_time.seconds != last_second)
            {
                last_second = cur_time.seconds;
                uint16_t x = cursor_col * GFX_CHAR_WIDTH;
                uint16_t y = cursor_row * GFX_CHAR_HEIGHT;
                
                // Toggle cursor visibility
                cursor_visible = !cursor_visible;
                
                if (cursor_visible)
                {
                    gfx_draw_cursor(x, y, COLOR_WHITE);
                }
                else
                {
                    gfx_erase_cursor(x, y);
                }
            }
        }
        
        // Polling - check for character
        c = 0;
        syscall(SYS_GETCHAR, (uint32_t)&c, 0, 0);
        
        if (!c)
        {
            if (!use_graphics_mode)
            {
                CursorPosition cp_refresh = {cursor_row, cursor_col};
                syscall(SYS_SET_CURSOR, (uint32_t)&cp_refresh, 0, 0);
            }
            continue;
        }

        // Got input - erase cursor before processing input
        if (use_graphics_mode && cursor_visible)
        {
            uint16_t x = cursor_col * GFX_CHAR_WIDTH;
            uint16_t y = cursor_row * GFX_CHAR_HEIGHT;
            gfx_erase_cursor(x, y);
            cursor_visible = false;
        }

        if (c == '\n' || c == '\r')
        {
            if (use_graphics_mode)
            {
                uint16_t x = cursor_col * GFX_CHAR_WIDTH;
                uint16_t y = cursor_row * GFX_CHAR_HEIGHT;
                gfx_erase_cursor(x, y);
            }
            break;
        }

        // Handle Ctrl+C - cancel current input
        if (c == CTRL_C)
        {
            // Print ^C
            if (use_graphics_mode)
            {
                uint16_t x = cursor_col * GFX_CHAR_WIDTH;
                uint16_t y = cursor_row * GFX_CHAR_HEIGHT;
                gfx_putchar(x, y, '^', COLOR_WHITE, COLOR_TRANSPARENT);
                screen_buffer_set(cursor_row, cursor_col, '^', COLOR_WHITE);
                cursor_col++;
                x = cursor_col * GFX_CHAR_WIDTH;
                gfx_putchar(x, y, 'C', COLOR_WHITE, COLOR_TRANSPARENT);
                screen_buffer_set(cursor_row, cursor_col, 'C', COLOR_WHITE);
            }
            else
            {
                CursorPosition cp = {cursor_row, cursor_col};
                syscall(SYS_PUTCHAR, (uint32_t)'^', 0x0F, (uint32_t)&cp);
                cursor_col++;
                cp.col = cursor_col;
                syscall(SYS_PUTCHAR, (uint32_t)'C', 0x0F, (uint32_t)&cp);
            }
            
            // Clear buffer and return 0 to indicate cancelled
            buffer[0] = '\0';
            
            // Move to next line
            cursor_col = 0;
            cursor_row++;
            if (cursor_row >= max_rows)
            {
                if (use_graphics_mode)
                {
                    screen_buffer_scroll();
                }
                else
                {
                    syscall(SYS_SCROLL, 0, 0, 0);
                }
                cursor_row = max_rows - 1;
            }
            
            return 0;  // Return 0 to indicate interrupt
        }

        if (c == ARROW_LEFT)
        {
            if (cursor_pos > 0)
            {
                cursor_pos--;
                if (cursor_col > 0)
                {
                    cursor_col--;
                }
                else if (cursor_row > start_row)
                {
                    cursor_row--;
                    cursor_col = max_cols - 1;
                }
                
                if (use_graphics_mode)
                {
                    uint16_t x = cursor_col * GFX_CHAR_WIDTH;
                    uint16_t y = cursor_row * GFX_CHAR_HEIGHT;
                    gfx_draw_cursor(x, y, COLOR_WHITE);
                    cursor_visible = true;
                }
                else
                {
                    CursorPosition cp = {cursor_row, cursor_col};
                    syscall(SYS_SET_CURSOR, (uint32_t)&cp, 0, 0);
                }
            }
        }
        // Arrow Right - move cursor right in input
        else if (c == ARROW_RIGHT)
        {
            if (cursor_pos < len)
            {
                cursor_pos++;
                cursor_col++;
                if (cursor_col >= max_cols)
                {
                    cursor_col = 0;
                    cursor_row++;
                }
                
                if (use_graphics_mode)
                {
                    uint16_t x = cursor_col * GFX_CHAR_WIDTH;
                    uint16_t y = cursor_row * GFX_CHAR_HEIGHT;
                    gfx_draw_cursor(x, y, COLOR_WHITE);
                    cursor_visible = true;
                }
                else
                {
                    CursorPosition cp = {cursor_row, cursor_col};
                    syscall(SYS_SET_CURSOR, (uint32_t)&cp, 0, 0);
                }
            }
        }
        // Arrow Up - previous command in history
        else if (c == ARROW_UP)
        {
            int max_history = history_count < HISTORY_SIZE ? history_count : HISTORY_SIZE;
            if (current_history_nav < max_history - 1)
            {
                // Save current input on first up press
                if (current_history_nav == -1)
                {
                    buffer[len] = '\0';
                    for (uint32_t i = 0; i <= len; i++)
                        temp_buffer[i] = buffer[i];
                }
                
                current_history_nav++;
                const char *hist_cmd = history_get(current_history_nav);
                if (hist_cmd)
                {
                    // Clear current input on screen
                    cursor_row = start_row;
                    cursor_col = start_col;
                    for (uint32_t i = 0; i < len; i++)
                    {
                        if (use_graphics_mode)
                        {
                            uint16_t x = cursor_col * GFX_CHAR_WIDTH;
                            uint16_t y = cursor_row * GFX_CHAR_HEIGHT;
                            screen_buffer_set(cursor_row, cursor_col, ' ', COLOR_WHITE);
                            gfx_restore_bg(x, y, GFX_CHAR_WIDTH + 1, GFX_CHAR_HEIGHT + 1);
                        }
                        else
                        {
                            CursorPosition cp = {cursor_row, cursor_col};
                            syscall(SYS_PUTCHAR, (uint32_t)' ', 0x0F, (uint32_t)&cp);
                        }
                        cursor_col++;
                        if (cursor_col >= max_cols)
                        {
                            cursor_col = 0;
                            cursor_row++;
                        }
                    }
                    
                    // Copy history command to buffer
                    len = strlen(hist_cmd);
                    for (uint32_t i = 0; i < len && i < max_len - 1; i++)
                        buffer[i] = hist_cmd[i];
                    cursor_pos = len;
                    
                    // Display new command
                    cursor_row = start_row;
                    cursor_col = start_col;
                    for (uint32_t i = 0; i < len; i++)
                    {
                        if (use_graphics_mode)
                        {
                            uint16_t x = cursor_col * GFX_CHAR_WIDTH;
                            uint16_t y = cursor_row * GFX_CHAR_HEIGHT;
                            gfx_putchar(x, y, buffer[i], COLOR_WHITE, COLOR_TRANSPARENT);
                            screen_buffer_set(cursor_row, cursor_col, buffer[i], COLOR_WHITE);
                        }
                        else
                        {
                            CursorPosition cp = {cursor_row, cursor_col};
                            syscall(SYS_PUTCHAR, (uint32_t)buffer[i], 0x0F, (uint32_t)&cp);
                        }
                        cursor_col++;
                        if (cursor_col >= max_cols)
                        {
                            cursor_col = 0;
                            cursor_row++;
                        }
                    }
                    
                    // Update cursor position
                    if (use_graphics_mode)
                    {
                        uint16_t x = cursor_col * GFX_CHAR_WIDTH;
                        uint16_t y = cursor_row * GFX_CHAR_HEIGHT;
                        gfx_draw_cursor(x, y, COLOR_WHITE);
                        cursor_visible = true;
                    }
                    else
                    {
                        CursorPosition cp = {cursor_row, cursor_col};
                        syscall(SYS_SET_CURSOR, (uint32_t)&cp, 0, 0);
                    }
                }
            }
        }
        // Arrow Down - next command in history (or back to current input)
        else if (c == ARROW_DOWN)
        {
            if (current_history_nav >= 0)
            {
                current_history_nav--;
                
                const char *new_cmd;
                uint32_t new_len;
                
                if (current_history_nav == -1)
                {
                    // Restore original input
                    new_cmd = temp_buffer;
                    new_len = strlen(temp_buffer);
                }
                else
                {
                    new_cmd = history_get(current_history_nav);
                    new_len = new_cmd ? strlen(new_cmd) : 0;
                }
                
                // Clear current input on screen
                cursor_row = start_row;
                cursor_col = start_col;
                for (uint32_t i = 0; i < len; i++)
                {
                    if (use_graphics_mode)
                    {
                        uint16_t x = cursor_col * GFX_CHAR_WIDTH;
                        uint16_t y = cursor_row * GFX_CHAR_HEIGHT;
                        screen_buffer_set(cursor_row, cursor_col, ' ', COLOR_WHITE);
                        gfx_restore_bg(x, y, GFX_CHAR_WIDTH + 1, GFX_CHAR_HEIGHT + 1);
                    }
                    else
                    {
                        CursorPosition cp = {cursor_row, cursor_col};
                        syscall(SYS_PUTCHAR, (uint32_t)' ', 0x0F, (uint32_t)&cp);
                    }
                    cursor_col++;
                    if (cursor_col >= max_cols)
                    {
                        cursor_col = 0;
                        cursor_row++;
                    }
                }
                
                // Copy new command to buffer
                len = new_len < max_len - 1 ? new_len : max_len - 1;
                for (uint32_t i = 0; i < len; i++)
                    buffer[i] = new_cmd[i];
                cursor_pos = len;
                
                // Display new command
                cursor_row = start_row;
                cursor_col = start_col;
                for (uint32_t i = 0; i < len; i++)
                {
                    if (use_graphics_mode)
                    {
                        uint16_t x = cursor_col * GFX_CHAR_WIDTH;
                        uint16_t y = cursor_row * GFX_CHAR_HEIGHT;
                        gfx_putchar(x, y, buffer[i], COLOR_WHITE, COLOR_TRANSPARENT);
                        screen_buffer_set(cursor_row, cursor_col, buffer[i], COLOR_WHITE);
                    }
                    else
                    {
                        CursorPosition cp = {cursor_row, cursor_col};
                        syscall(SYS_PUTCHAR, (uint32_t)buffer[i], 0x0F, (uint32_t)&cp);
                    }
                    cursor_col++;
                    if (cursor_col >= max_cols)
                    {
                        cursor_col = 0;
                        cursor_row++;
                    }
                }
                
                // Update cursor position
                if (use_graphics_mode)
                {
                    uint16_t x = cursor_col * GFX_CHAR_WIDTH;
                    uint16_t y = cursor_row * GFX_CHAR_HEIGHT;
                    gfx_draw_cursor(x, y, COLOR_WHITE);
                    cursor_visible = true;
                }
                else
                {
                    CursorPosition cp = {cursor_row, cursor_col};
                    syscall(SYS_SET_CURSOR, (uint32_t)&cp, 0, 0);
                }
            }
        }
        else if (c == '\b' || c == 127)
        { // Backspace
            if (cursor_pos > 0)
            {
                // Shift characters left from cursor position
                for (uint32_t i = cursor_pos - 1; i < len - 1; i++)
                    buffer[i] = buffer[i + 1];
                len--;
                cursor_pos--;
                
                // Move cursor back
                if (cursor_col > 0)
                {
                    cursor_col--;
                }
                else if (cursor_row > start_row)
                {
                    cursor_row--;
                    cursor_col = max_cols - 1;
                }
                
                // Redraw from cursor position to end
                uint32_t save_row = cursor_row;
                uint32_t save_col = cursor_col;
                for (uint32_t i = cursor_pos; i <= len; i++)
                {
                    char ch = (i < len) ? buffer[i] : ' ';
                    if (use_graphics_mode)
                    {
                        uint16_t x = cursor_col * GFX_CHAR_WIDTH;
                        uint16_t y = cursor_row * GFX_CHAR_HEIGHT;
                        if (ch == ' ')
                        {
                            screen_buffer_set(cursor_row, cursor_col, ' ', COLOR_WHITE);
                            gfx_restore_bg(x, y, GFX_CHAR_WIDTH + 1, GFX_CHAR_HEIGHT + 1);
                        }
                        else
                        {
                            gfx_putchar(x, y, ch, COLOR_WHITE, COLOR_TRANSPARENT);
                            screen_buffer_set(cursor_row, cursor_col, ch, COLOR_WHITE);
                        }
                    }
                    else
                    {
                        CursorPosition cp = {cursor_row, cursor_col};
                        syscall(SYS_PUTCHAR, (uint32_t)ch, 0x0F, (uint32_t)&cp);
                    }
                    cursor_col++;
                    if (cursor_col >= max_cols)
                    {
                        cursor_col = 0;
                        cursor_row++;
                    }
                }
                
                // Restore cursor position
                cursor_row = save_row;
                cursor_col = save_col;
                
                if (use_graphics_mode)
                {
                    uint16_t x = cursor_col * GFX_CHAR_WIDTH;
                    uint16_t y = cursor_row * GFX_CHAR_HEIGHT;
                    gfx_draw_cursor(x, y, COLOR_WHITE);
                    cursor_visible = true;
                }
                else
                {
                    CursorPosition cp = {cursor_row, cursor_col};
                    syscall(SYS_SET_CURSOR, (uint32_t)&cp, 0, 0);
                }
            }
        }
        else if (c >= 32 && c <= 126)
        {
            // Insert character at cursor position
            if (len < max_len - 1)
            {
                // Shift characters right to make room
                for (uint32_t i = len; i > cursor_pos; i--)
                    buffer[i] = buffer[i - 1];
                buffer[cursor_pos] = c;
                len++;
                cursor_pos++;
                
                // Display character and redraw rest of line
                if (use_graphics_mode)
                {
                    uint16_t x = cursor_col * GFX_CHAR_WIDTH;
                    uint16_t y = cursor_row * GFX_CHAR_HEIGHT;
                    gfx_putchar(x, y, c, COLOR_WHITE, COLOR_TRANSPARENT);
                    screen_buffer_set(cursor_row, cursor_col, c, COLOR_WHITE);
                }
                else
                {
                    CursorPosition cp = {cursor_row, cursor_col};
                    syscall(SYS_PUTCHAR, (uint32_t)c, 0x0F, (uint32_t)&cp);
                }
                
                cursor_col++;
                if (cursor_col >= max_cols)
                {
                    cursor_col = 0;
                    cursor_row++;
                    if (cursor_row >= max_rows)
                    {
                        if (use_graphics_mode)
                        {
                            screen_buffer_scroll();
                        }
                        else
                        {
                            syscall(SYS_SCROLL, 0, 0, 0);
                        }
                        cursor_row = max_rows - 1;
                        start_row--;
                    }
                }
                
                // Redraw characters after cursor position (for insert mode)
                uint32_t save_row = cursor_row;
                uint32_t save_col = cursor_col;
                for (uint32_t i = cursor_pos; i < len; i++)
                {
                    if (use_graphics_mode)
                    {
                        uint16_t px = cursor_col * GFX_CHAR_WIDTH;
                        uint16_t py = cursor_row * GFX_CHAR_HEIGHT;
                        gfx_putchar(px, py, buffer[i], COLOR_WHITE, COLOR_TRANSPARENT);
                        screen_buffer_set(cursor_row, cursor_col, buffer[i], COLOR_WHITE);
                    }
                    else
                    {
                        CursorPosition cp = {cursor_row, cursor_col};
                        syscall(SYS_PUTCHAR, (uint32_t)buffer[i], 0x0F, (uint32_t)&cp);
                    }
                    cursor_col++;
                    if (cursor_col >= max_cols)
                    {
                        cursor_col = 0;
                        cursor_row++;
                    }
                }
                cursor_row = save_row;
                cursor_col = save_col;
                
                // Update cursor position
                if (use_graphics_mode)
                {
                    uint16_t new_x = cursor_col * GFX_CHAR_WIDTH;
                    uint16_t new_y = cursor_row * GFX_CHAR_HEIGHT;
                    gfx_draw_cursor(new_x, new_y, COLOR_WHITE);
                    cursor_visible = true;
                }
                else
                {
                    CursorPosition cp2 = {cursor_row, cursor_col};
                    syscall(SYS_SET_CURSOR, (uint32_t)&cp2, 0, 0);
                }
            }
        }
    }

    buffer[len] = '\0';
    
    // Move to next line
    cursor_col = 0;
    cursor_row++;
    if (cursor_row >= max_rows)
    {
        if (use_graphics_mode)
        {
            screen_buffer_scroll();
        }
        else
        {
            syscall(SYS_SCROLL, 0, 0, 0);
        }
        cursor_row = max_rows - 1;
    }
    
    // Update cursor position (text mode only)
    if (!use_graphics_mode)
    {
        CursorPosition cp_final = {cursor_row, cursor_col};
        syscall(SYS_SET_CURSOR, (uint32_t)&cp_final, 0, 0);
    }

    return len;
}

/**
 * Parse a quoted string, handling escape sequences
 * Input: pointer to opening quote, output buffer and max length
 * Returns: pointer to character after closing quote
 */
char *parse_quoted_string(char *input, char *output, uint32_t max_len)
{
    if (*input != '"')
    {
        return input;
    }

    input++; // Skip opening quote
    uint32_t len = 0;

    while (*input && *input != '"' && len < max_len - 1)
    {
        output[len++] = *input++;
    }

    output[len] = '\0';

    if (*input == '"')
    {
        input++; // Skip closing quote
    }

    return input;
}

/**
 * Parse multiple arguments from command line
 * Handles both quoted and non-quoted arguments
 * Stores arguments in an array and returns the count
 */
uint32_t parse_multiple_arguments(const char *args_str, char (*arguments)[256], uint32_t max_args)
{
    uint32_t arg_count = 0;
    uint32_t i = 0;
    uint32_t cmd_len = strlen(args_str);

    while (i < cmd_len && arg_count < max_args)
    {
        // Skip spaces
        while (i < cmd_len && args_str[i] == ' ')
        {
            i++;
        }

        if (i >= cmd_len)
            break;

        // Check if argument is quoted
        if (args_str[i] == '"')
        {
            // Parse quoted argument
            char *after = parse_quoted_string((char *)&args_str[i], arguments[arg_count], 256);
            i = after - args_str;
            arg_count++;
        }
        else
        {
            // Parse non-quoted argument
            uint32_t j = 0;
            while (i < cmd_len && args_str[i] != ' ' && j < 255)
            {
                arguments[arg_count][j++] = args_str[i++];
            }
            arguments[arg_count][j] = '\0';
            arg_count++;
        }
    }

    return arg_count;
}

// ========================= PATH HELPER FUNCTIONS =========================

/**
 * @brief Resolve a path string starting from root (inode 1)
 * @param path The path to resolve (e.g., "/bin" or "bin")
 * @return The inode of the directory, or 0 if not found
 */
static uint32_t resolve_path_from_root(const char *path)
{
    if (!path || strlen(path) == 0)
    {
        return 1; // Return root
    }

    uint32_t working_inode = 1; // Start from root

    // Skip leading slash
    const char *p = path;
    if (*p == '/')
    {
        p++;
    }

    if (*p == '\0')
    {
        return 1; // Just "/" means root
    }

    // Parse path components
    char component[256];
    while (*p)
    {
        // Extract next component
        uint32_t i = 0;
        while (*p && *p != '/' && i < 255)
        {
            component[i++] = *p++;
        }
        component[i] = '\0';

        if (i == 0)
        {
            // Empty component (double slash), skip
            if (*p == '/')
                p++;
            continue;
        }

        // Skip slash for next iteration
        if (*p == '/')
            p++;

        // Try to find this component in current directory
        uint8_t dir_data[BLOCK_SIZE * 4];
        struct EXT2DriverRequest request = {
            .buf = dir_data,
            .name = component,
            .name_len = strlen(component),
            .parent_inode = working_inode,
            .buffer_size = BLOCK_SIZE * 4,
            .is_directory = true};

        int32_t retcode = 0;
        syscall(SYS_READ_DIR, (uint32_t)&request, (uint32_t)&retcode, 0);

        if (retcode != 0)
        {
            return 0; // Directory not found
        }

        // Get inode from first entry (which should be the "." entry of the found dir)
        struct EXT2DirectoryEntry *entry = (struct EXT2DirectoryEntry *)dir_data;
        if (entry->inode != 0)
        {
            working_inode = entry->inode;
        }
        else
        {
            return 0; // Not found
        }
    }

    return working_inode;
}

/**
 * @brief Check if a file exists in a directory by scanning entries
 * @param filename Name of the file to find
 * @param dir_inode Inode of the directory to search in
 * @return true if file exists, false otherwise
 */
static bool file_exists_in_dir(const char *filename, uint32_t dir_inode)
{
    // Read the directory contents
    uint8_t dir_buf[BLOCK_SIZE * 4];
    struct EXT2DriverRequest request = {
        .buf = dir_buf,
        .parent_inode = dir_inode,
        .buffer_size = BLOCK_SIZE * 4,
        .is_directory = true};

    int32_t retcode = 0;
    syscall(SYS_READ_DIR, (uint32_t)&request, (uint32_t)&retcode, 0);

    if (retcode != 0)
    {
        return false;
    }

    // Scan directory entries
    uint32_t offset = 0;
    uint32_t filename_len = strlen(filename);

    while (offset < BLOCK_SIZE * 4)
    {
        struct EXT2DirectoryEntry *entry = (struct EXT2DirectoryEntry *)&dir_buf[offset];

        if (entry->inode == 0 || entry->rec_len == 0)
        {
            break;
        }

        // Check if entry name matches
        if (entry->name_len == filename_len)
        {
            char *entry_name = (char *)entry + sizeof(struct EXT2DirectoryEntry);
            bool match = true;
            for (uint32_t i = 0; i < filename_len; i++)
            {
                if (entry_name[i] != filename[i])
                {
                    match = false;
                    break;
                }
            }
            if (match)
            {
                return true; // File found
            }
        }

        offset += entry->rec_len;
    }

    return false;
}

/**
 * @brief Find an executable in PATH directories
 * @param cmd Command name to find
 * @param found_inode Pointer to store the directory inode if found
 * @return true if found, false otherwise
 */
static bool find_in_path(const char *cmd, uint32_t *found_inode)
{
    for (uint32_t i = 0; i < path_count; i++)
    {
        // Resolve path directory to inode
        uint32_t dir_inode = resolve_path_from_root(path_dirs[i]);
        if (dir_inode == 0)
            continue;

        // Check if file exists in this directory
        if (file_exists_in_dir(cmd, dir_inode))
        {
            *found_inode = dir_inode;
            return true;
        }
    }
    return false;
}

/**
 * @brief Add a directory to PATH
 * @param dir Directory path to add
 * @return 0 on success, -1 if PATH is full, -2 if already exists
 */
static int8_t path_add(const char *dir)
{
    if (!dir || strlen(dir) == 0)
    {
        return -1;
    }

    // Check if already exists
    for (uint32_t i = 0; i < path_count; i++)
    {
        if (memcmp(path_dirs[i], dir, strlen(dir)) == 0 &&
            strlen(path_dirs[i]) == strlen(dir))
        {
            return -2; // Already exists
        }
    }

    if (path_count >= MAX_PATH_DIRS)
    {
        return -1; // PATH is full
    }

    // Add to PATH
    uint32_t len = strlen(dir);
    if (len >= MAX_PATH_LEN)
        len = MAX_PATH_LEN - 1;
    memcpy(path_dirs[path_count], dir, len);
    path_dirs[path_count][len] = '\0';
    path_count++;

    return 0;
}

/**
 * @brief Remove a directory from PATH
 * @param dir Directory path to remove
 * @return 0 on success, -1 if not found
 */
static int8_t path_remove(const char *dir)
{
    if (!dir || strlen(dir) == 0)
    {
        return -1;
    }

    for (uint32_t i = 0; i < path_count; i++)
    {
        if (memcmp(path_dirs[i], dir, strlen(dir)) == 0 &&
            strlen(path_dirs[i]) == strlen(dir))
        {
            // Shift remaining entries
            for (uint32_t j = i; j < path_count - 1; j++)
            {
                memcpy(path_dirs[j], path_dirs[j + 1], MAX_PATH_LEN);
            }
            path_count--;
            return 0;
        }
    }
    return -1; // Not found
}

/**
 * @brief List all directories in PATH
 */
static void path_list(void)
{
    print_string_color("PATH=", COLOR_CYAN);
    if (path_count == 0)
    {
        print_string("(empty)");
    }
    else
    {
        for (uint32_t i = 0; i < path_count; i++)
        {
            if (i > 0)
                print_string(":");
            print_string(path_dirs[i]);
        }
    }
    print_string("\n");
    flush_output_buffer();
}

// Compose and persist environment into INIT_FILE_NAME
static int8_t init_save(void)
{
    char content[4096];
    uint32_t idx = 0;

    // export PATH=...
    const char *path_prefix = "export PATH=";
    uint32_t prefix_len = strlen(path_prefix);
    memcpy(&content[idx], path_prefix, prefix_len); idx += prefix_len;
    for (uint32_t i = 0; i < path_count; i++)
    {
        uint32_t len = strlen(path_dirs[i]);
        if (idx + len + 2 >= sizeof(content)) break;
        memcpy(&content[idx], path_dirs[i], len); idx += len;
        if (i + 1 < path_count) content[idx++] = ':';
    }
    content[idx++] = '\n';

    // export BG=...
    const char *bg_prefix = "export BG=";
    uint32_t bg_prefix_len = strlen(bg_prefix);
    memcpy(&content[idx], bg_prefix, bg_prefix_len); idx += bg_prefix_len;
    uint32_t bg_len = strlen(bg_env);
    if (bg_len + idx + 2 < sizeof(content))
    {
        memcpy(&content[idx], bg_env, bg_len); idx += bg_len;
    }
    content[idx++] = '\n';
    content[idx] = '\0';

    // Delete old init then write new
    int32_t retcode = 0;
    struct EXT2DriverRequest del_req = {
        .name = INIT_FILE_NAME,
        .name_len = strlen(INIT_FILE_NAME),
        .parent_inode = 1,
        .is_directory = false};
    syscall(SYS_DELETE, (uint32_t)&del_req, (uint32_t)&retcode, 0);

    struct EXT2DriverRequest write_req = {
        .buf = (uint8_t *)content,
        .name = INIT_FILE_NAME,
        .name_len = strlen(INIT_FILE_NAME),
        .parent_inode = 1,
        .buffer_size = idx,
        .is_directory = false};
    syscall(SYS_WRITE, (uint32_t)&write_req, (uint32_t)&retcode, 0);
    return (int8_t)retcode;
}

// Compatibility: keep existing callers using path_save name
static int8_t path_save(void)
{
    return init_save();
}

// ========================= INIT SCRIPT SUPPORT =========================

static void init_build_default(char *out_buf, uint32_t *out_len)
{
    const char *defaults =
        "export PATH=/:/bin\n"
        "export BG=/bg\n";
    uint32_t len = strlen(defaults);
    memcpy(out_buf, defaults, len);
    *out_len = len;
}

static void init_apply_line(const char *line)
{
    // Trim trailing newline/carriage return
    char tmp[512];
    uint32_t len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
        len--;
    if (len == 0)
        return;
    if (len >= sizeof(tmp))
        len = sizeof(tmp) - 1;
    memcpy(tmp, line, len);
    tmp[len] = '\0';

    // Only support export lines for now
    if (len >= 6 && memcmp(tmp, "export", 6) == 0)
    {
        // Reuse existing export handling by constructing args
        char argbuf[512] = {0};
        const char *p = tmp + 6;
        while (*p == ' ')
            p++;
        if (*p == '\0')
            return;
        // Single argument after export
        memcpy(argbuf, p, strlen(p));
        // Call export logic path via execute_command short-circuit
        // (We can't call execute_command directly because it expects full parsing)
        // Instead, we handle BG and PATH here directly
        if (memcmp(argbuf, "PATH=", 5) == 0)
        {
            // Simulate export PATH=...
            char *dirs = &argbuf[5];
            path_count = 0;
            char *q = dirs;
            while (*q)
            {
                char dir[MAX_PATH_LEN];
                uint32_t i = 0;
                while (*q && *q != ':' && i < MAX_PATH_LEN - 1)
                {
                    dir[i++] = *q++;
                }
                dir[i] = '\0';
                if (i > 0)
                    path_add(dir);
                if (*q == ':')
                    q++;
            }
        }
        else if (memcmp(argbuf, "PATH+=", 6) == 0)
        {
            path_add(&argbuf[6]);
        }
        else if (memcmp(argbuf, "PATH-=", 6) == 0)
        {
            path_remove(&argbuf[6]);
        }
        else if (memcmp(argbuf, "BG=", 3) == 0)
        {
            uint32_t vlen = strlen(&argbuf[3]);
            if (vlen < BG_ENV_MAX)
            {
                memcpy(bg_env, &argbuf[3], vlen);
                bg_env[vlen] = '\0';
            }
        }
        else if (memcmp(argbuf, "BG+=", 4) == 0)
        {
            const char *value = &argbuf[4];
            uint32_t cur_len = strlen(bg_env);
            uint32_t add_len = strlen(value);
            uint32_t needed = cur_len + (cur_len > 0 ? 1 : 0) + add_len;
            if (needed < BG_ENV_MAX)
            {
                if (cur_len > 0)
                {
                    bg_env[cur_len] = ':';
                    cur_len++;
                }
                memcpy(&bg_env[cur_len], value, add_len);
                bg_env[cur_len + add_len] = '\0';
            }
        }
    }
}

static void init_load_or_create(void)
{
    // Try to read existing init file
    uint8_t buf[1024];
    struct EXT2DriverRequest read_req = {
        .buf = buf,
        .name = INIT_FILE_NAME,
        .name_len = strlen(INIT_FILE_NAME),
        .parent_inode = 1,
        .buffer_size = sizeof(buf),
        .is_directory = false};

    int32_t retcode = 0;
    syscall(SYS_READ, (uint32_t)&read_req, (uint32_t)&retcode, 0);

    if (retcode == 0)
    {
        // Parse lines
        char *p = (char *)buf;
        while (*p)
        {
            char *line_end = p;
            while (*line_end && *line_end != '\n')
                line_end++;
            char saved = *line_end;
            *line_end = '\0';
            init_apply_line(p);
            *line_end = saved;
            if (*line_end == '\n')
                line_end++;
            p = line_end;
        }
        return;
    }

    // If not found, create with defaults
    char content[512];
    uint32_t content_len = 0;
    init_build_default(content, &content_len);

    struct EXT2DriverRequest write_req = {
        .buf = (uint8_t *)content,
        .name = INIT_FILE_NAME,
        .name_len = strlen(INIT_FILE_NAME),
        .parent_inode = 1,
        .buffer_size = content_len,
        .is_directory = false};

    syscall(SYS_WRITE, (uint32_t)&write_req, (uint32_t)&retcode, 0);
    // Apply defaults locally too
    init_apply_line("export PATH=/:/bin");
    init_apply_line("export BG=/bg");
}

// ========================= END PATH FUNCTIONS ============================

/**
 * Parse and execute a command
 */
int8_t execute_command(char *cmd_line, uint32_t *current_inode)
{
    if (strlen(cmd_line) == 0)
    {
        return 0;
    }

    char cmd[50] = {0};
    char arg1[256] = {0};
    char arg2[256] = {0};
    char args_rest[512] = {0}; // For commands like echo that need all remaining args
    int i = 0, j = 0;
    int cmd_len = strlen(cmd_line);

    // Parse command name
    while (i < cmd_len && cmd_line[i] != ' ' && j < 49)
    {
        cmd[j++] = cmd_line[i++];
    }
    cmd[j] = '\0';

    // Skip spaces after command
    int args_start = i;
    while (i < cmd_len && cmd_line[i] == ' ')
    {
        i++;
        args_start = i;
    }

    // Parse first argument - handle quotes
    j = 0;
    if (cmd_line[i] == '"')
    {
        // Quoted argument
        char *after_quote = parse_quoted_string(&cmd_line[i], arg1, 256);
        i = after_quote - cmd_line;
    }
    else
    {
        // Non-quoted argument
        while (i < cmd_len && cmd_line[i] != ' ' && j < 255)
        {
            arg1[j++] = cmd_line[i++];
        }
        arg1[j] = '\0';
    }

    // Skip spaces
    while (i < cmd_len && cmd_line[i] == ' ')
    {
        i++;
    }

    // Parse second argument - handle quotes
    j = 0;
    if (cmd_line[i] == '"')
    {
        // Quoted argument
        char *after_quote = parse_quoted_string(&cmd_line[i], arg2, 256);
        i = after_quote - cmd_line;
    }
    else
    {
        // Non-quoted argument
        while (i < cmd_len && cmd_line[i] != ' ' && j < 255)
        {
            arg2[j++] = cmd_line[i++];
        }
        arg2[j] = '\0';
    }

    // For echo and similar commands, get all remaining args
    if (args_start < cmd_len)
    {
        j = 0;
        int rest_i = args_start;
        while (rest_i < cmd_len && j < 511)
        {
            args_rest[j++] = cmd_line[rest_i++];
        }
        args_rest[j] = '\0';
    }

    // Execute command
    int8_t retcode = -1;

    if (memcmp(cmd, "cd", 2) == 0 && strlen(cmd) == 2)
    {
        // cd only accepts 1 argument (can be quoted)
        retcode = cd_command(arg1, current_inode);
        if (retcode != 0)
        {
            print_error("cd: directory not found");
        }
    }
    else if (memcmp(cmd, "ls", 2) == 0 && strlen(cmd) == 2)
    {
        retcode = ls_command(strlen(arg1) > 0 ? arg1 : 0, *current_inode);
        if (retcode != 0)
        {
            print_error("ls: error reading directory");
        }
    }
    else if (memcmp(cmd, "mkdir", 5) == 0 && strlen(cmd) == 5)
    {
        if (strlen(args_rest) > 0)
        {
            // Parse multiple directory names from args_rest
            char dir_names[10][256] = {0};
            uint32_t dir_count = parse_multiple_arguments(args_rest, dir_names, 10);

            if (dir_count > 0)
            {
                for (uint32_t i = 0; i < dir_count; i++)
                {
                    retcode = mkdir_command(dir_names[i], *current_inode);
                    if (retcode != 0)
                    {
                        print_error("mkdir: cannot create directory");
                    }
                }
            }
            else
            {
                print_error("mkdir: missing argument");
            }
        }
        else
        {
            print_error("mkdir: missing argument");
        }
    }
    else if (memcmp(cmd, "cat", 3) == 0 && strlen(cmd) == 3)
    {
        if (strlen(arg1) > 0)
        {
            retcode = cat_command(arg1, *current_inode);
            if (retcode != 0)
            {
                print_error("cat: file not found or error reading");
            }
        }
        else
        {
            print_error("cat: missing argument");
        }
    }
    else if (memcmp(cmd, "rm", 2) == 0 && strlen(cmd) == 2)
    {
        if (strlen(args_rest) > 0)
        {
            // Check if -r flag is present
            bool recursive = false;
            char args_to_parse[512] = {0};

            if (memcmp(args_rest, "-r ", 3) == 0 || memcmp(args_rest, "-r", 2) == 0)
            {
                recursive = true;
                // Skip the -r flag and any spaces
                uint32_t i = 0;
                while (args_rest[i] && (args_rest[i] == '-' || args_rest[i] == 'r' || args_rest[i] == ' '))
                {
                    i++;
                }
                // Copy remaining arguments
                uint32_t j = 0;
                while (args_rest[i] && j < 510)
                {
                    args_to_parse[j++] = args_rest[i++];
                }
                args_to_parse[j] = '\0';
            }
            else
            {
                memcpy(args_to_parse, args_rest, strlen(args_rest));
            }

            // Parse multiple file names
            char file_names[20][256] = {0};
            uint32_t file_count = parse_multiple_arguments(args_to_parse, file_names, 20);

            if (file_count > 0)
            {
                for (uint32_t i = 0; i < file_count; i++)
                {
                    retcode = rm_command(file_names[i], *current_inode, recursive);
                    if (retcode != 0)
                    {
                        if (retcode == 1)
                        {
                            print_error("rm: file not found");
                        }
                        else if (retcode == 2)
                        {
                            print_error("rm: directory not empty (use rm -r to remove recursively)");
                        }
                        else
                        {
                            print_error("rm: cannot remove file");
                        }
                    }
                }
            }
            else
            {
                print_error("rm: missing argument");
            }
        }
        else
        {
            print_error("rm: missing argument");
        }
    }
    else if (memcmp(cmd, "cp", 2) == 0 && strlen(cmd) == 2)
    {
        if (strlen(arg1) > 0 && strlen(arg2) > 0)
        {
            retcode = cp_command(arg1, arg2, *current_inode);
            if (retcode != 0)
            {
                print_error("cp: error copying file");
            }
        }
        else
        {
            print_error("cp: missing arguments");
        }
    }
    else if (memcmp(cmd, "mv", 2) == 0 && strlen(cmd) == 2)
    {
        if (strlen(arg1) > 0 && strlen(arg2) > 0)
        {
            retcode = mv_command(arg1, arg2, *current_inode);
            if (retcode != 0)
            {
                print_error("mv: error moving file");
            }
        }
        else
        {
            print_error("mv: missing arguments");
        }
    }
    else if (memcmp(cmd, "find", 4) == 0 && strlen(cmd) == 4)
    {
        if (strlen(args_rest) > 0)
        {
            // Parse find arguments: find <path> or find <path> -name <filename>
            char find_args[3][256] = {0};
            uint32_t find_arg_count = parse_multiple_arguments(args_rest, find_args, 3);

            if (find_arg_count >= 3)
            {
                // Format: find <path> -name <filename>
                char *search_path = find_args[0];
                char *flag = find_args[1];
                char *search_name = find_args[2];

                if (memcmp(flag, "-name", 5) == 0 && strlen(flag) == 5)
                {
                    retcode = find_command(search_path, search_name, *current_inode);
                }
                else
                {
                    print_error("find: invalid syntax (use: find <path> or find <path> -name <filename>)");
                }
            }
            else if (find_arg_count == 1)
            {
                // Check if it's a path or filename
                char *arg = find_args[0];
                if ((memcmp(arg, ".", 1) == 0 && strlen(arg) == 1) ||
                    (memcmp(arg, "/", 1) == 0 && strlen(arg) == 1) ||
                    (arg[0] == '.' && arg[1] == '/') ||
                    (arg[0] == '/'))
                {
                    // It's a path: find <path> (list all files in that directory)
                    retcode = find_command(arg, "*", *current_inode);
                }
                else
                {
                    // Backward compatibility: find <filename> (search from root)
                    retcode = find_command("/", arg, *current_inode);
                }
            }
            else
            {
                print_error("find: missing arguments (use: find <path> or find <path> -name <filename>)");
            }
        }
        else
        {
            print_error("find: missing argument");
        }
    }
    else if (memcmp(cmd, "grep", 4) == 0 && strlen(cmd) == 4)
    {
        if (strlen(arg1) > 0 && strlen(arg2) > 0)
        {
            retcode = grep_command(arg1, arg2, *current_inode);
            if (retcode != 0)
            {
                print_error("grep: file not found");
            }
        }
        else
        {
            print_error("grep: missing arguments");
        }
    }
    else if (memcmp(cmd, "touch", 5) == 0 && strlen(cmd) == 5)
    {
        if (strlen(args_rest) > 0)
        {
            // Check for redirect operator >
            char redirect_parts[2][512] = {0};
            uint32_t part_count = parse_redirect(args_rest, redirect_parts, 2);

            if (part_count == 2 && strlen(redirect_parts[1]) > 0)
            {
                // Has redirect: extract path and filename
                char path_str[512] = {0};
                char filename_str[256] = {0};
                memcpy(path_str, redirect_parts[1], strlen(redirect_parts[1]));

                // Find last slash to separate directory path from filename
                int last_slash = -1;
                for (int i = strlen(path_str) - 1; i >= 0; i--)
                {
                    if (path_str[i] == '/')
                    {
                        last_slash = i;
                        break;
                    }
                }

                uint32_t target_inode = *current_inode;
                if (last_slash >= 0)
                {
                    // Has directory path
                    path_str[last_slash] = '\0';
                    memcpy(filename_str, &path_str[last_slash + 1], strlen(&path_str[last_slash + 1]));

                    // Create path and get target inode
                    target_inode = create_path_and_return_inode(path_str, *current_inode);
                    if (target_inode == 0)
                    {
                        print_error("touch: failed to create directory path");
                    }
                    else
                    {
                        // Create file in target directory
                        if (strlen(filename_str) > 0)
                        {
                            retcode = touch_command(filename_str, target_inode);
                            if (retcode != 0)
                            {
                                print_error("touch: cannot create file");
                            }
                        }
                        else
                        {
                            print_error("touch: invalid filename");
                        }
                    }
                }
                else
                {
                    // No directory path, just filename
                    memcpy(filename_str, path_str, strlen(path_str));
                    retcode = touch_command(filename_str, *current_inode);
                    if (retcode != 0)
                    {
                        print_error("touch: cannot create file");
                    }
                }
            }
            else
            {
                // Normal touch: create files in current directory
                char touch_args[20][256] = {0};
                uint32_t touch_arg_count = parse_multiple_arguments(args_rest, touch_args, 20);

                if (touch_arg_count > 0)
                {
                    for (uint32_t i = 0; i < touch_arg_count; i++)
                    {
                        retcode = touch_command(touch_args[i], *current_inode);
                        if (retcode != 0)
                        {
                            print_error("touch: cannot create file");
                        }
                    }
                }
                else
                {
                    print_error("touch: missing argument");
                }
            }
        }
        else
        {
            print_error("touch: missing argument");
        }
    }
    else if (memcmp(cmd, "echo", 4) == 0 && strlen(cmd) == 4)
    {
        // Check for redirect operator >
        char *redirect_pos = NULL;
        char *temp = args_rest;
        int in_quote = 0;
        while (*temp)
        {
            if (*temp == '"')
            {
                in_quote = !in_quote;
            }
            else if (*temp == '>' && !in_quote)
            {
                redirect_pos = temp;
                break;
            }
            temp++;
        }

        if (redirect_pos)
        {
            // Extract text and filename
            *redirect_pos = '\0'; // Null terminate text at >
            char *text = args_rest;
            char *filename_start = redirect_pos + 1;
            char parsed_filename[256] = {0};

            // Skip spaces before filename
            while (*filename_start == ' ')
            {
                filename_start++;
            }

            // Parse filename (handle quotes)
            if (*filename_start == '"')
            {
                parse_quoted_string(filename_start, parsed_filename, 256);
            }
            else
            {
                // Non-quoted filename
                char *temp_ptr = filename_start;
                int fn_len = 0;
                while (*temp_ptr && *temp_ptr != ' ' && fn_len < 255)
                {
                    parsed_filename[fn_len++] = *temp_ptr++;
                }
                parsed_filename[fn_len] = '\0';
            }

            // Trim spaces from end of text
            int text_len = strlen(text) - 1;
            while (text_len >= 0 && text[text_len] == ' ')
            {
                text[text_len] = '\0';
                text_len--;
            }

            if (strlen(parsed_filename) > 0 && strlen(text) > 0)
            {
                retcode = echo_redirect_command(text, parsed_filename, *current_inode);
                if (retcode != 0)
                {
                    print_error("echo: error writing to file");
                }
            }
            else
            {
                print_error("echo: invalid redirect syntax");
            }
        }
        else
        {
            retcode = echo_command(strlen(args_rest) > 0 ? args_rest : "");
        }
    }
    else if (memcmp(cmd, "clear", 5) == 0 && strlen(cmd) == 5)
    {
        // Clear screen - restore wallpaper
        if (use_graphics_mode)
        {
            screen_buffer_clear();
            gfx_draw_wallpaper();
        }
        else
        {
            syscall(SYS_CLEAR_FRAMEBUFFER, 0, 0, 0);
        }
        cursor_row = 0;
        cursor_col = 0;
        retcode = 0;
    }
    else if (memcmp(cmd, "badapple", 8) == 0 && strlen(cmd) == 8)
    {
        // Run Bad Apple animation in kernel mode
        // Clear screen first
        if (use_graphics_mode)
        {
            screen_buffer_clear();
        }
        
        // Run the animation via syscall (runs in kernel mode)
        // Animation can be stopped with CTRL+C
        int32_t result = 0;
        syscall(SYS_BADAPPLE, (uint32_t)&result, 0, 0);
        
        // After animation ends, restore wallpaper and reset cursor
        if (use_graphics_mode)
        {
            screen_buffer_clear();
            gfx_draw_wallpaper();
        }
        cursor_row = 0;
        cursor_col = 0;
        retcode = (int8_t)result;
    }
    else if (memcmp(cmd, "ps", 2) == 0 && strlen(cmd) == 2)
    {
        // List running processes
        retcode = ps_command();
    }
    else if (memcmp(cmd, "kill", 4) == 0 && strlen(cmd) == 4)
    {
        // Kill process by PID
        if (strlen(arg1) > 0)
        {
            // Parse PID from arg1
            uint32_t pid = 0;
            for (uint32_t i = 0; arg1[i] >= '0' && arg1[i] <= '9'; i++)
            {
                pid = pid * 10 + (arg1[i] - '0');
            }
            retcode = kill_command(pid);
        }
        else
        {
            print_error("kill: usage: kill <pid>");
            retcode = -1;
        }
    }
    else if (memcmp(cmd, "bg", 2) == 0 && strlen(cmd) == 2)
    {
        // Change wallpaper/background
        retcode = bg_command(strlen(arg1) > 0 ? arg1 : NULL);
    }
    else if (memcmp(cmd, "heapinfo", 8) == 0 && strlen(cmd) == 8)
    {
        // Display heap memory statistics
        UserHeapStats stats;
        heap_stats(&stats);
        
        print_string_color("Heap Memory Statistics:\n", COLOR_CYAN);
        print_string_color("===================================\n", COLOR_CYAN);
        
        // Helper to print numbers
        char num_buf[16];
        
        print_string("Total size:     ");
        uint32_t val = stats.total_size;
        int idx = 0;
        if (val == 0) { num_buf[idx++] = '0'; }
        else { char tmp[16]; int ti = 0; while (val) { tmp[ti++] = '0' + (val % 10); val /= 10; } while (ti--) num_buf[idx++] = tmp[ti]; }
        num_buf[idx] = '\0';
        print_string(num_buf);
        print_string(" bytes\n");
        
        print_string("Used size:      ");
        val = stats.used_size; idx = 0;
        if (val == 0) { num_buf[idx++] = '0'; }
        else { char tmp[16]; int ti = 0; while (val) { tmp[ti++] = '0' + (val % 10); val /= 10; } while (ti--) num_buf[idx++] = tmp[ti]; }
        num_buf[idx] = '\0';
        print_string(num_buf);
        print_string(" bytes\n");
        
        print_string("Free size:      ");
        val = stats.free_size; idx = 0;
        if (val == 0) { num_buf[idx++] = '0'; }
        else { char tmp[16]; int ti = 0; while (val) { tmp[ti++] = '0' + (val % 10); val /= 10; } while (ti--) num_buf[idx++] = tmp[ti]; }
        num_buf[idx] = '\0';
        print_string(num_buf);
        print_string(" bytes\n");
        
        print_string("Block count:    ");
        val = stats.block_count; idx = 0;
        if (val == 0) { num_buf[idx++] = '0'; }
        else { char tmp[16]; int ti = 0; while (val) { tmp[ti++] = '0' + (val % 10); val /= 10; } while (ti--) num_buf[idx++] = tmp[ti]; }
        num_buf[idx] = '\0';
        print_string(num_buf);
        print_string("\n");
        
        print_string("Free blocks:    ");
        val = stats.free_block_count; idx = 0;
        if (val == 0) { num_buf[idx++] = '0'; }
        else { char tmp[16]; int ti = 0; while (val) { tmp[ti++] = '0' + (val % 10); val /= 10; } while (ti--) num_buf[idx++] = tmp[ti]; }
        num_buf[idx] = '\0';
        print_string(num_buf);
        print_string("\n");
        
        print_string("Allocations:    ");
        val = stats.alloc_count; idx = 0;
        if (val == 0) { num_buf[idx++] = '0'; }
        else { char tmp[16]; int ti = 0; while (val) { tmp[ti++] = '0' + (val % 10); val /= 10; } while (ti--) num_buf[idx++] = tmp[ti]; }
        num_buf[idx] = '\0';
        print_string(num_buf);
        print_string("\n");
        
        print_string("Frees:          ");
        val = stats.free_count; idx = 0;
        if (val == 0) { num_buf[idx++] = '0'; }
        else { char tmp[16]; int ti = 0; while (val) { tmp[ti++] = '0' + (val % 10); val /= 10; } while (ti--) num_buf[idx++] = tmp[ti]; }
        num_buf[idx] = '\0';
        print_string(num_buf);
        print_string("\n");
        
        flush_output_buffer();
        retcode = 0;
    }
    else if (memcmp(cmd, "heaptest", 8) == 0 && strlen(cmd) == 8)
    {
        // Test heap allocation and freeing (runs in kernel mode)
        print_string_color("Testing Heap Memory Allocation...\n", COLOR_CYAN);
        print_string_color("===================================\n", COLOR_CYAN);
        
        HeapTestResult result;
        heap_test(&result);
        
        // Test 1
        print_string("[1] Allocating 64 bytes... ");
        if (result.test1_alloc) {
            print_string_color("OK\n", COLOR_GREEN);
        } else {
            print_string_color("FAILED\n", COLOR_RED);
        }
        
        // Test 2
        print_string("[2] Allocating 128 bytes... ");
        if (result.test2_alloc) {
            print_string_color("OK\n", COLOR_GREEN);
        } else {
            print_string_color("FAILED\n", COLOR_RED);
        }
        
        // Test 3
        print_string("[3] Allocating 32 bytes (zeroed)... ");
        if (result.test3_calloc) {
            print_string_color("OK\n", COLOR_GREEN);
            print_string("    Zero-initialized: ");
            if (result.test3_zeroed) {
                print_string_color("YES\n", COLOR_GREEN);
            } else {
                print_string_color("NO\n", COLOR_RED);
            }
        } else {
            print_string_color("FAILED\n", COLOR_RED);
        }
        
        // Test 4
        print_string("[4] Freeing first buffer... ");
        if (result.test4_free) {
            print_string_color("OK\n", COLOR_GREEN);
        } else {
            print_string_color("FAILED\n", COLOR_RED);
        }
        
        // Test 5
        print_string("[5] Allocating 32 bytes (reuse)... ");
        if (result.test5_realloc) {
            print_string_color("OK\n", COLOR_GREEN);
        } else {
            print_string_color("FAILED\n", COLOR_RED);
        }
        
        // Test 6
        print_string("[6] Cleaning up... ");
        if (result.test6_cleanup) {
            print_string_color("OK\n", COLOR_GREEN);
        } else {
            print_string_color("FAILED\n", COLOR_RED);
        }
        
        // Show final stats
        print_string_color("\nFinal heap stats:\n", COLOR_CYAN);
        char num_buf[16];
        uint32_t val;
        int idx;
        
        print_string("  Allocations: ");
        val = result.alloc_count; idx = 0;
        if (val == 0) { num_buf[idx++] = '0'; }
        else { char tmp[16]; int ti = 0; while (val) { tmp[ti++] = '0' + (val % 10); val /= 10; } while (ti--) num_buf[idx++] = tmp[ti]; }
        num_buf[idx] = '\0';
        print_string(num_buf);
        print_string(", Frees: ");
        val = result.free_count; idx = 0;
        if (val == 0) { num_buf[idx++] = '0'; }
        else { char tmp[16]; int ti = 0; while (val) { tmp[ti++] = '0' + (val % 10); val /= 10; } while (ti--) num_buf[idx++] = tmp[ti]; }
        num_buf[idx] = '\0';
        print_string(num_buf);
        print_string("\n");
        
        flush_output_buffer();
        retcode = 0;
    }
    else if (memcmp(cmd, "help", 4) == 0 && strlen(cmd) == 4)
    {
        // Display available commands
        print_string_color("Available Commands:\n", COLOR_CYAN);
        print_string_color("===================================\n", COLOR_CYAN);
        print_string_color("Navigation:\n", COLOR_YELLOW);
        print_string("  cd <dir>      - Change directory\n");
        print_string("  ls            - List directory contents\n");
        print_string_color("File Operations:\n", COLOR_YELLOW);
        print_string("  cat <file>    - Display file contents\n");
        print_string("  touch <file>  - Create empty file (use touch >path/file to auto-create path)\n");
        print_string("  cp <src> <dst>- Copy file\n");
        print_string("  mv <src> <dst>- Move/rename file\n");
        print_string("  rm [-r] <f>   - Remove file (add -r for directories)\n");
        print_string("  mkdir <dir>   - Create directory\n");
        print_string_color("Search:\n", COLOR_YELLOW);
        print_string("  find <path> [-name <file>] - Find files\n");
        print_string("  grep <pat> <f>- Search pattern in file\n");
        print_string_color("Process:\n", COLOR_YELLOW);
        print_string("  ps            - List processes\n");
        print_string("  kill <pid>    - Kill process by PID\n");
        print_string_color("Memory:\n", COLOR_YELLOW);
        print_string("  heapinfo      - Show heap statistics\n");
        print_string("  heaptest      - Test heap alloc/free\n");
        print_string_color("Other:\n", COLOR_YELLOW);
        print_string("  echo <text> [>file] - Print text (supports redirect)\n");
        print_string("  exec <prog>   - Execute program\n");
        print_string("  clear         - Clear screen\n");
        print_string("  bg [next|prev]- Change wallpaper\n");
        print_string("  badapple      - Play Bad Apple animation\n");
        print_string("  beep          - Play a short beep\n");
        print_string("  help          - Show this help\n");
        print_string_color("Environment:\n", COLOR_YELLOW);
        print_string("  export        - Show/modify PATH variable\n");
        print_string("  export PATH=/a:/b - Set PATH\n");
        print_string("  export PATH+=/dir - Add directory to PATH\n");
        print_string("  export PATH-=/dir - Remove directory from PATH\n");
        print_string("  export BG=...  - Set wallpaper list (colon separated)\n");
        print_string("  export BG+=... - Append wallpaper entries\n");
        print_string_color("Shortcuts:\n", COLOR_YELLOW);
        print_string("  Arrow Up/Down - Command history\n");
        print_string("  Arrow L/R     - Move cursor\n");
        print_string("  Ctrl+C        - Cancel input\n");
        flush_output_buffer();
        retcode = 0;
    }
    else if (memcmp(cmd, "export", 6) == 0 && strlen(cmd) == 6)
    {
        // PATH management command
        if (strlen(arg1) == 0)
        {
            // Show current PATH
            path_list();
            print_string_color("BG=", COLOR_CYAN);
            if (strlen(bg_env) == 0)
            {
                print_string("(empty)\n");
            }
            else
            {
                print_string(bg_env);
                print_string("\n");
            }
            flush_output_buffer();
            retcode = 0;
        }
        else if (memcmp(arg1, "PATH+=", 6) == 0)
        {
            // Append to PATH
            char *dir = &arg1[6];
            retcode = path_add(dir);
            if (retcode == 0)
            {
                init_save(); // Persist changes
                print_string_color("Added to PATH: ", COLOR_GREEN);
                print_string(dir);
                print_string("\n");
                flush_output_buffer();
            }
            else if (retcode == -2)
            {
                print_error("export: directory already in PATH");
            }
            else
            {
                print_error("export: failed to add to PATH");
            }
        }
        else if (memcmp(arg1, "PATH-=", 6) == 0)
        {
            // Remove from PATH
            char *dir = &arg1[6];
            retcode = path_remove(dir);
            if (retcode == 0)
            {
                init_save(); // Persist changes
                print_string_color("Removed from PATH: ", COLOR_YELLOW);
                print_string(dir);
                print_string("\n");
                flush_output_buffer();
            }
            else
            {
                print_error("export: directory not found in PATH");
            }
        }
        else if (memcmp(arg1, "PATH=", 5) == 0)
        {
            // Set PATH (clear and add all directories separated by :)
            char *dirs = &arg1[5];
            
            // Clear current PATH
            path_count = 0;
            
            // Parse colon-separated directories
            char *p = dirs;
            while (*p)
            {
                char dir[MAX_PATH_LEN];
                uint32_t i = 0;
                while (*p && *p != ':' && i < MAX_PATH_LEN - 1)
                {
                    dir[i++] = *p++;
                }
                dir[i] = '\0';
                
                if (i > 0)
                {
                    path_add(dir);
                }
                
                if (*p == ':')
                    p++;
            }
            
            path_save(); // Persist changes
            print_string_color("PATH updated\n", COLOR_GREEN);
            path_list();
            retcode = 0;
        }
        else if (memcmp(arg1, "BG=", 3) == 0)
        {
            const char *value = &arg1[3];
            uint32_t len = strlen(value);
            if (len >= BG_ENV_MAX)
            {
                print_error("export: BG value too long");
                retcode = -1;
            }
            else
            {
                memcpy(bg_env, value, len);
                bg_env[len] = '\0';
                print_string_color("BG set to: ", COLOR_GREEN);
                print_string(strlen(bg_env) ? bg_env : "(empty)");
                print_string("\n");
                flush_output_buffer();
                init_save();
                retcode = 0;
            }
        }
        else if (memcmp(arg1, "BG+=", 4) == 0)
        {
            const char *value = &arg1[4];
            uint32_t cur_len = strlen(bg_env);
            uint32_t add_len = strlen(value);
            uint32_t needed = cur_len + (cur_len > 0 ? 1 : 0) + add_len;
            if (needed >= BG_ENV_MAX)
            {
                print_error("export: BG value too long");
                retcode = -1;
            }
            else
            {
                if (cur_len > 0)
                {
                    bg_env[cur_len] = ':';
                    cur_len++;
                }
                memcpy(&bg_env[cur_len], value, add_len);
                bg_env[cur_len + add_len] = '\0';
                print_string_color("BG updated: ", COLOR_GREEN);
                print_string(bg_env);
                print_string("\n");
                flush_output_buffer();
                init_save();
                retcode = 0;
            }
        }
        else if (memcmp(arg1, "BG", 2) == 0 && strlen(arg1) == 2)
        {
            print_string_color("BG=", COLOR_CYAN);
            print_string(strlen(bg_env) ? bg_env : "(empty)");
            print_string("\n");
            flush_output_buffer();
            retcode = 0;
        }
        else
        {
            print_error("export: invalid syntax");
            print_string("Usage:\n");
            print_string("  export                - Show current PATH\n");
            print_string("  export PATH=/dir1:/dir2 - Set PATH\n");
            print_string("  export PATH+=/dir     - Add directory to PATH\n");
            print_string("  export PATH-=/dir     - Remove directory from PATH\n");
            flush_output_buffer();
            retcode = -1;
        }
    }
    else if (memcmp(cmd, "exec", 4) == 0 && strlen(cmd) == 4)
    {
        if (strlen(arg1) > 0)
        {
            retcode = exec_command(arg1, *current_inode);
            if (retcode != 0)
            {
                print_error("exec: error executing program");
            }
        }
        else
        {
            print_error("exec: missing argument");
        }
    }
    else if (memcmp(cmd, "beep", 4) == 0 && strlen(cmd) == 4)
    {
        // Simple beep sound
        syscall(SYS_BEEP, 0, 0, 0);
        retcode = 0;
    }
    else if (strlen(cmd) > 0)
    {
        // Try to execute as a program (look for executable in current directory)
        retcode = exec_command(cmd, *current_inode);
        if (retcode != 0)
        {
            // Not found in current directory, search PATH directories
            uint32_t path_inode = 0;
            if (find_in_path(cmd, &path_inode))
            {
                // Found in PATH, execute from there
                retcode = exec_command((char *)cmd, path_inode);
                if (retcode != 0)
                {
                    print_error("exec: error executing program");
                }
            }
            else
            {
                print_error("command not found");
            }
        }
    }

    return retcode;
}

int main(void)
{
    // Initialize graphics mode if enabled
    if (use_graphics_mode)
    {
        gfx_init();
        // Draw wallpaper as background
        gfx_draw_wallpaper();
    }

    // Initialize filesystem - open shell directory
    uint8_t buf[BLOCK_SIZE * 64];
    struct EXT2DriverRequest request = {
        .buf = buf,
        .name = "shell",
        .parent_inode = 1,
        .buffer_size = BLOCK_SIZE * 64,
        .name_len = 5,
        .is_directory = true
    };
    int32_t retcode = 0;
    syscall(SYS_READ_DIR, (uint32_t)&request, (uint32_t)&retcode, 0);

    // Load/init environment from init script (creates defaults if missing)
    init_load_or_create();

    uint32_t current_inode = 1; // Start at root directory (inode 1)
    char input_buffer[INPUT_BUFFER_SIZE];

    // REPL loop
    while (1)
    {
        print_cwd(current_inode);
        flush_output_buffer(); // Flush prompt before waiting for input

        uint32_t len = read_line(input_buffer, INPUT_BUFFER_SIZE);

        if (len > 0)
        {
            // Add command to history
            history_add(input_buffer);
            
            // Check for pipe operator
            char pipe_commands[5][512] = {0};
            uint32_t pipe_count = parse_pipe(input_buffer, pipe_commands, 5);

            if (pipe_count > 1)
            {
                // Has pipe - need to chain commands
                // Strategy: first command writes to temp file, next command reads from it

                char temp_file[256] = "__pipe_temp__";

                for (uint32_t i = 0; i < pipe_count; i++)
                {
                    char cmd_line[512] = {0};
                    memcpy(cmd_line, pipe_commands[i], strlen(pipe_commands[i]));

                    if (i < pipe_count - 1)
                    {
                        // Not the last command - redirect output to temp file
                        // Check if it's echo command - use pipe mode
                        char *cmd_start = cmd_line;
                        while (*cmd_start && *cmd_start == ' ')
                            cmd_start++;

                        if (memcmp(cmd_start, "echo", 4) == 0 && (cmd_start[4] == ' ' || cmd_start[4] == '\0'))
                        {
                            // This is echo command - extract text and use echo_pipe_command
                            // Format: echo "text" or echo text... (handles mixed quoted/unquoted)
                            char *args_pos = &cmd_start[4];
                            while (*args_pos && *args_pos == ' ')
                                args_pos++;

                            // Parse ALL arguments (quoted and unquoted mixed)
                            char text[512] = {0};
                            int text_idx = 0;
                            int in_quote = 0;

                            for (char *p = args_pos; *p && text_idx < 511; p++)
                            {
                                if (*p == '"')
                                {
                                    in_quote = !in_quote;
                                    continue; // Skip the quote character itself
                                }

                                if (*p == ' ' && !in_quote)
                                {
                                    // Space outside quotes - include it
                                    text[text_idx++] = ' ';
                                }
                                else
                                {
                                    // Any other character - include it
                                    text[text_idx++] = *p;
                                }
                            }
                            text[text_idx] = '\0';

                            // Use echo_pipe_command which writes to file without screen output
                            echo_pipe_command(text, temp_file, current_inode);
                        }
                        else
                        {
                            // Not echo, check for redirect and add if needed
                            char *redirect_pos = NULL;
                            int in_quote = 0;
                            for (char *p = cmd_line; *p; p++)
                            {
                                if (*p == '"')
                                    in_quote = !in_quote;
                                if (*p == '>' && !in_quote)
                                {
                                    redirect_pos = p;
                                    break;
                                }
                            }

                            if (!redirect_pos)
                            {
                                // No redirect, add one to temp file
                                uint32_t cmd_len = strlen(cmd_line);
                                cmd_line[cmd_len++] = ' ';
                                cmd_line[cmd_len++] = '>';
                                cmd_line[cmd_len++] = ' ';
                                memcpy(&cmd_line[cmd_len], temp_file, strlen(temp_file));
                                cmd_line[cmd_len + strlen(temp_file)] = '\0';
                            }

                            execute_command(cmd_line, &current_inode);
                        }
                    }
                    else
                    {
                        // Last command - check if it's grep and add temp_file as argument
                        char *cmd_start = cmd_line;
                        while (*cmd_start && *cmd_start == ' ')
                            cmd_start++;

                        if (memcmp(cmd_start, "grep", 4) == 0 && (cmd_start[4] == ' ' || cmd_start[4] == '\0'))
                        {
                            // This is grep command - add temp_file as the file to search
                            uint32_t cmd_len = strlen(cmd_line);
                            cmd_line[cmd_len++] = ' ';
                            memcpy(&cmd_line[cmd_len], temp_file, strlen(temp_file));
                            cmd_line[cmd_len + strlen(temp_file)] = '\0';

                            execute_command(cmd_line, &current_inode);
                        }
                        else
                        {
                            execute_command(cmd_line, &current_inode);
                        }
                    }
                }

                // Clean up temp file after pipe execution
                struct EXT2DriverRequest cleanup_request = {
                    .buf = NULL,
                    .name = temp_file,
                    .name_len = strlen(temp_file),
                    .parent_inode = current_inode,
                    .buffer_size = 0,
                    .is_directory = false};
                int32_t cleanup_retcode = 0;
                syscall(SYS_DELETE, (uint32_t)&cleanup_request, (uint32_t)&cleanup_retcode, 0);
            }
            else
            {
                // No pipe, execute single command
                execute_command(input_buffer, &current_inode);
            }

            // Flush output buffer after command execution
            flush_output_buffer();
        }
    }

    return 0;
}
