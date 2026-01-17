#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// VGA Settings
#define VGA_WIDTH   320
#define VGA_HEIGHT  200
#define VGA_MEMORY ((uint8_t*)0xC00A0000) // Higher-half mapping for 0xA0000

// Common colors in VGA palette
#define COLOR_BLACK       0x00
#define COLOR_BLUE        0x01
#define COLOR_GREEN       0x02
#define COLOR_CYAN        0x03
#define COLOR_RED         0x04
#define COLOR_MAGENTA     0x05
#define COLOR_BROWN       0x06
#define COLOR_LIGHT_GRAY  0x07
#define COLOR_DARK_GRAY   0x08
#define COLOR_LIGHT_BLUE  0x09
#define COLOR_LIGHT_GREEN 0x0A
#define COLOR_LIGHT_CYAN  0x0B
#define COLOR_LIGHT_RED   0x0C
#define COLOR_PINK        0x0D
#define COLOR_YELLOW      0x0E
#define COLOR_WHITE       0x0F

/**
 * Initialize VGA graphics mode (320x200 with 256 colors).
 * This will switch the display from text mode to graphics mode.
 */
void graphics_initialize(void);

/**
 * Switch back to text mode (80x25).
 */
void graphics_exit(void);

/**
 * Plot a single pixel at (x,y) with specified color.
 * 
 * @param x X coordinate (0 to VGA_WIDTH-1)
 * @param y Y coordinate (0 to VGA_HEIGHT-1)
 * @param color Color value (0-255)
 */
void graphics_pixel(uint16_t x, uint16_t y, uint8_t color);

/**
 * Clear the screen with a specified color.
 *
 * @param color Color value (0-255)
 */
void graphics_clear(uint8_t color);

/**
 * Draw a single character at the specified position using the 8x8 font.
 * Uses the font lookup table from paste.txt.
 *
 * @param x X coordinate
 * @param y Y coordinate
 * @param c Character to draw
 * @param color Foreground color (0-255)
 * @param bgcolor Background color (0-255)
 */
void graphics_char(uint16_t x, uint16_t y, unsigned char c, uint8_t color, uint8_t bgcolor);

/**
 * Draw a string at the specified position using the 8x8 font.
 * Uses the font lookup table from paste.txt.
 *
 * @param x X coordinate
 * @param y Y coordinate
 * @param str String to draw
 * @param color Foreground color (0-255)
 * @param bgcolor Background color (0-255)
 */
void graphics_string(uint16_t x, uint16_t y, const char* str, uint8_t color, uint8_t bgcolor);

/**
 * Set the cursor position to specific coordinates
 * 
 * @param x X coordinate
 * @param y Y coordinate
 */
void graphics_set_cursor(uint16_t x, uint16_t y);

/**
 * Get the current cursor position
 * 
 * @param x Pointer to store X coordinate
 * @param y Pointer to store Y coordinate
 */
void graphics_get_cursor(uint16_t *x, uint16_t *y);

/**
 * Scroll the cursor by the specified amounts, ensuring it remains within bounds
 * 
 * @param dx X direction change (can be negative)
 * @param dy Y direction change (can be negative)
 */
void graphics_scroll_cursor(int16_t dx, int16_t dy);

/**
 * Move the cursor by the specified amounts, clamping at boundaries
 * 
 * @param dx X direction change (can be negative)
 * @param dy Y direction change (can be negative)
 */
void graphics_move_cursor(int16_t dx, int16_t dy);

/**
 * Fill a rectangular area with a specified color
 * 
 * @param x X coordinate
 * @param y Y coordinate
 * @param width Width of the rectangle
 * @param height Height of the rectangle
 * @param color Color value (0-255)
 */
void graphics_rect_fill(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t color);

/**
 * Draw a rectangle outline at the specified position
 * 
 * @param x X coordinate
 * @param y Y coordinate
 * @param width Width of the rectangle
 * @param height Height of the rectangle
 * @param color Color value (0-255)
 */
void graphics_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t color);

/**
 * Fill the entire screen with all available colors
 * 
 * @param color Color value
*/
void graphics_fill_screen_with_color();

/**
 * Draw the cursor at the current position
 */
void graphics_draw_cursor(void);

/**
 * Erase the cursor at the current position
 */
void graphics_erase_cursor(void);

/**
 * Store the character at the current cursor position for blinking
 * 
 * @param c Character to store
 */
void graphics_store_char_at_cursor(char c);

/**
 * Set the colors for the cursor
 * 
 * @param fg_color Foreground color (0-255)
 * @param bg_color Background color (0-255)
 */
void graphics_set_cursor_colors(uint8_t fg_color, uint8_t bg_color);

/**
 * Set the cursor blink 
 * 
 */
void graphics_blink_cursor(void);

/**
 * Scroll the screen up by a specified number of lines
 * @param lines Number of lines to scroll (1-200)
 * @param color Color to fill the newly exposed lines (0-255)
 */
void graphics_scroll(uint16_t lines, uint8_t color);

/**
 * Draw a line from (x1, y1) to (x2, y2) using Bresenham's algorithm
 * 
 * @param x1 Starting X coordinate
 * @param y1 Starting Y coordinate
 * @param x2 Ending X coordinate
 * @param y2 Ending Y coordinate
 * @param color Color value (0-255)
 */
void graphics_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color);

/**
 * Draw a filled circle at the specified center with given radius
 * 
 * @param center_x Center X coordinate
 * @param center_y Center Y coordinate
 * @param radius Radius of the circle
 * @param color Color value (0-255)
 */
void graphics_circle_filled(uint16_t center_x, uint16_t center_y, uint16_t radius, uint8_t color);

/**
 * Draw the wallpaper to fill the entire screen
 * Uses the embedded wallpaper_data from wallpaper.h
 */
void graphics_draw_wallpaper(void);

/**
 * Redraw a specific region of the wallpaper
 * Useful for restoring background after text/graphics are erased
 * 
 * @param x X coordinate of the region
 * @param y Y coordinate of the region
 * @param width Width of the region
 * @param height Height of the region
 */
void graphics_draw_wallpaper_region(uint16_t x, uint16_t y, uint16_t width, uint16_t height);

/**
 * Get the wallpaper pixel color at a specific position
 * Useful for transparent text rendering
 * 
 * @param x X coordinate
 * @param y Y coordinate
 * @return Color value (0-255) from wallpaper
 */
uint8_t graphics_get_wallpaper_pixel(uint16_t x, uint16_t y);

/**
 * Draw a character with transparent background (uses wallpaper)
 * This function is only available in kernel mode.
 * 
 * @param x X coordinate
 * @param y Y coordinate
 * @param c Character to draw
 * @param color Foreground color (0-255)
 */
void graphics_char_transparent(uint16_t x, uint16_t y, unsigned char c, uint8_t color);

/**
 * Draw a string with transparent background (uses wallpaper)
 * This function is only available in kernel mode.
 * 
 * @param x X coordinate
 * @param y Y coordinate
 * @param str String to draw
 * @param color Foreground color (0-255)
 */
void graphics_string_transparent(uint16_t x, uint16_t y, const char* str, uint8_t color);

/**
 * Scroll the screen with transparent background (restores wallpaper)
 * This function is only available in kernel mode.
 * 
 * @param lines Number of lines to scroll (1-200)
 */
void graphics_scroll_transparent(uint16_t lines);

#endif
