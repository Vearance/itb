#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/cpu/portio.h"

void framebuffer_set_cursor(uint16_t r, uint16_t c) {
    uint16_t pos = r * FRAMEBUFFER_WIDTH + c;
    out(CURSOR_PORT_CMD, 0x0F);
    out(CURSOR_PORT_DATA, (uint16_t)(pos & 0xFF));
    out(CURSOR_PORT_CMD, 0x0E);
    out(CURSOR_PORT_DATA, (uint16_t)((pos >> 8) & 0xFF));
}

void framebuffer_write(uint16_t row, uint16_t col, char c, uint8_t fg, uint8_t bg) {
    // Only write valid printable ASCII characters and newline
    // Replace invalid characters with space
    if (c < 32 || c > 126) {
        c = ' ';
    }
    
    uint16_t pos = (row * FRAMEBUFFER_WIDTH + col) * 2;
    FRAMEBUFFER_MEMORY_OFFSET[pos] = c;
    FRAMEBUFFER_MEMORY_OFFSET[pos + 1] = (bg << 4) | (fg & 0x0F);
}

char framebuffer_read(uint16_t row, uint16_t col) {
    uint16_t pos = (row * FRAMEBUFFER_WIDTH + col) * 2;
    return FRAMEBUFFER_MEMORY_OFFSET[pos];
}

void framebuffer_clear(void) {
    for (uint16_t i = 0; i < FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT; i++) {
        framebuffer_write(i / FRAMEBUFFER_WIDTH, i % FRAMEBUFFER_WIDTH, 0x00, 0x07, 0x00);
    }
}
