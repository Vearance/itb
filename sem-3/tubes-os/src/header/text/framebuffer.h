#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define FRAMEBUFFER_MEMORY_OFFSET ((uint8_t*) 0xC00B8000)
#define CURSOR_PORT_CMD    0x03D4
#define CURSOR_PORT_DATA   0x03D5

#define FRAMEBUFFER_WIDTH  80
#define FRAMEBUFFER_HEIGHT 25

typedef struct {
    uint32_t row;
    uint32_t col;
} CursorPosition;

/**
 * Terminal framebuffer
 * Resolution: 80x25
 * Starting at FRAMEBUFFER_MEMORY_OFFSET,
 * - Even number memory: Character, 8-bit
 * - Odd number memory:  Character color lower 4-bit, Background color upper 4-bit
*/

/**
 * Set framebuffer character and color with corresponding parameter values.
 * More details: https://en.wikipedia.org/wiki/BIOS_color_attributes
 *
 * @param row Vertical location (index start 0)
 * @param col Horizontal location (index start 0)
 * @param c   Character
 * @param fg  Foreground / Character color
 * @param bg  Background color
 */
void framebuffer_write(uint16_t row, uint16_t col, char c, uint8_t fg, uint8_t bg);

/**
 * Set cursor to specified location. Row and column starts from 0
 * 
 * @param r row
 * @param c column
*/
void framebuffer_set_cursor(uint16_t r, uint16_t c);

/**
 * Read character from framebuffer at specified location
 * 
 * @param row Vertical location (index start 0)
 * @param col Horizontal location (index start 0)
 * @return Character at the specified location
 */
char framebuffer_read(uint16_t row, uint16_t col);

/**
 * Set all cell in framebuffer character to 0x00 (empty character)
 * and color to 0x07 (gray character & black background)
 * Extra note: It's allowed to use different color palette for this
 *
 */
void framebuffer_clear(void);

#endif