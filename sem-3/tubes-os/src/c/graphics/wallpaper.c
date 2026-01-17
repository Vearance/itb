/**
 * Wallpaper rendering functions for HuTaOS
 * This file is compiled only for the kernel, not for user programs.
 * Supports multiple wallpapers with runtime switching.
 */

#include "../../header/graphics/graphics.h"
#include "../../header/graphics/wallpaper.h"
#include "../../header/graphics/font.h"
#include "../../header/driver/ext2.h"
#include "../../header/stdlib/string.h"

// Current active wallpaper index (only index 0 is valid after FS-backed load)
uint32_t current_wallpaper_index = 0;

// Single wallpaper buffer loaded from filesystem
static uint8_t wallpaper_buffer[WALLPAPER_SIZE];
static char current_wallpaper_name[WALLPAPER_NAME_MAX] = "Solid Black";
static uint32_t wallpaper_count = 1;

const uint8_t wallpaper_black[64000] = {[0 ... 63999] = 0x00};

// Initialize wallpaper system
void wallpaper_init(void) {
    // Start with a solid black wallpaper in buffer
    memcpy(wallpaper_buffer, wallpaper_black, WALLPAPER_SIZE);
    current_wallpaper_index = 0;
    wallpaper_count = 1;
}

uint32_t wallpaper_get_count(void) {
    return wallpaper_count;
}

uint32_t wallpaper_get_current(void) {
    return current_wallpaper_index;
}

bool wallpaper_set_current(uint32_t index) {
    // Only a single wallpaper slot is maintained when loading from filesystem
    if (index != 0) return false;
    current_wallpaper_index = 0;
    return true;
}

const char* wallpaper_get_name(uint32_t index) {
    (void)index; // single slot
    return current_wallpaper_name;
}

const uint8_t* wallpaper_get_data(uint32_t index) {
    (void)index; // single slot
    return wallpaper_buffer;
}

const uint8_t* wallpaper_get_current_data(void) {
    return wallpaper_get_data(current_wallpaper_index);
}

void graphics_draw_wallpaper(void) {
    // Draw current active wallpaper from embedded data
    const uint8_t *data = wallpaper_get_current_data();
    for (uint32_t i = 0; i < WALLPAPER_WIDTH * WALLPAPER_HEIGHT; i++) {
        VGA_MEMORY[i] = data[i];
    }
}

int wallpaper_load_from_fs(uint32_t parent_inode, const char *filename) {
    if (filename == NULL) {
        return 1; // invalid argument
    }

    uint32_t name_len = strlen(filename);
    if (name_len == 0 || name_len > 255) {
        return 1; // invalid name length
    }

    struct EXT2DriverRequest req = {
        .buf = wallpaper_buffer,
        .name = (char *)filename,
        .name_len = name_len,
        .parent_inode = parent_inode,
        .buffer_size = WALLPAPER_SIZE,
        .is_directory = false
    };

    int8_t ret = read(req);
    if (ret != 0) {
        return ret; // propagate FS error codes
    }

    // Update state and name (truncate if needed)
    uint32_t copy_len = name_len < (WALLPAPER_NAME_MAX - 1) ? name_len : (WALLPAPER_NAME_MAX - 1);
    memcpy(current_wallpaper_name, filename, copy_len);
    current_wallpaper_name[copy_len] = '\0';
    current_wallpaper_index = 0;
    wallpaper_count = 1;
    return 0;
}

void graphics_draw_wallpaper_region(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    // Redraw a specific region of the current wallpaper (for restoring background)
    const uint8_t *data = wallpaper_get_current_data();
    for (uint16_t dy = 0; dy < height; dy++) {
        for (uint16_t dx = 0; dx < width; dx++) {
            uint16_t px = x + dx;
            uint16_t py = y + dy;
            if (px < VGA_WIDTH && py < VGA_HEIGHT) {
                uint32_t offset = py * VGA_WIDTH + px;
                VGA_MEMORY[offset] = data[offset];
            }
        }
    }
}

uint8_t graphics_get_wallpaper_pixel(uint16_t x, uint16_t y) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) {
        return 0;
    }
    const uint8_t *data = wallpaper_get_current_data();
    return data[y * VGA_WIDTH + x];
}

void graphics_char_transparent(uint16_t x, uint16_t y, unsigned char c, uint8_t color) {
    if (c >= 128) return;

    const uint8_t* char_data = lookup[(uint8_t)c];
    uint8_t size = char_data[0];

    // Draw with wallpaper as background
    for (uint8_t row = 0; row < 8; row++) {
        for (uint8_t col = 0; col < 5; col++) {
            uint8_t bg = graphics_get_wallpaper_pixel(x + col, y + row);
            graphics_pixel(x + col, y + row, bg);
        }
    }

    // Draw foreground pixels
    for (uint8_t i = 1; i < size; i++) {
        uint8_t data = char_data[i];
        uint8_t col = (data >> 4) & 0x0F;  
        uint8_t row = data & 0x0F;      

        if (col < 5 && row < 8) {
            graphics_pixel(x + col, y + row, color);
        }
    }
}

void graphics_string_transparent(uint16_t x, uint16_t y, const char* str, uint8_t color) {
    uint16_t current_x = x;
    uint16_t current_y = y;
    
    for (uint32_t i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\n') {
            current_x = x;
            current_y += 8;
            continue;
        }
        graphics_char_transparent(current_x, current_y, str[i], color);
        current_x += 5;  // 5 pixels char width
        if (current_x >= VGA_WIDTH - 5) {
            current_x = x;
            current_y += 8;
        }
    }
}

void graphics_scroll_transparent(uint16_t lines) {
    // Reserve last row for clock (8 pixels high)
    uint16_t scrollable_height = VGA_HEIGHT - 8;  // 200 - 8 = 192 pixels
    
    // For transparent mode with wallpaper, we can't just move pixels
    // because the wallpaper would get distorted.
    // Instead, restore the wallpaper for the entire scrollable area.
    // Text will need to be redrawn by the caller.
    // 
    // However, since shell doesn't have a text buffer, we'll do a simpler approach:
    // Just restore wallpaper for the newly exposed lines at bottom.
    // The caller should manage what text to keep.
    
    (void)lines; // Unused in simple approach
    
    // Restore wallpaper for scrollable area (preserving clock row)
    graphics_draw_wallpaper_region(0, 0, VGA_WIDTH, scrollable_height);
}
