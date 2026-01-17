#include "../../header/graphics/graphics.h"
#include "../../header/graphics/font.h"
#include "../../header/cpu/portio.h"
#include "../../header/stdlib/string.h"

// VGA Register definitions
#define MISC_OUT_REG        0x3C2
#define CRT_ADDR_REG        0x3D4
#define CRT_DATA_REG        0x3D5
#define SEQ_ADDR_REG        0x3C4
#define SEQ_DATA_REG        0x3C5
#define GRAP_ADDR_REG       0x3CE
#define GRAP_DATA_REG       0x3CF
#define ATTR_ADDR_REG       0x3C0
#define ATTR_DATA_REG       0x3C0
#define DAC_WRITE_ADDR      0x3C8
#define DAC_DATA_REG        0x3C9
#define INPUT_STATUS_1      0x3DA

// Define cursor appearance
#define CURSOR_HEIGHT 8
#define CURSOR_WIDTH 1

// Cursor state
static uint16_t cursor_x = 0;
static uint16_t cursor_y = 0;
static uint8_t cursor_fg_color = COLOR_PINK;
static uint8_t cursor_bg_color = COLOR_BLACK;
static char cursor_char = 0;
static bool cursor_visible = true;

void graphics_initialize(void) {
    // Miscellaneous output register
    out(MISC_OUT_REG, 0x63);
    
    // Disable CRTC protection
    out(CRT_ADDR_REG, 0x11);
    out(CRT_DATA_REG, in(CRT_DATA_REG) & 0x7F);

    // Sequencer registers - first do sequencer reset
    out(SEQ_ADDR_REG, 0x00); out(SEQ_DATA_REG, 0x03);  // Reset
    out(SEQ_ADDR_REG, 0x01); out(SEQ_DATA_REG, 0x01);  // Clocking Mode
    out(SEQ_ADDR_REG, 0x02); out(SEQ_DATA_REG, 0x0F);  // Map Mask
    out(SEQ_ADDR_REG, 0x03); out(SEQ_DATA_REG, 0x00);  // Character Map Select
    out(SEQ_ADDR_REG, 0x04); out(SEQ_DATA_REG, 0x0E);  // Memory Mode
    
    // End sequencer reset
    out(SEQ_ADDR_REG, 0x00); out(SEQ_DATA_REG, 0x03);

    // CRTC registers for mode 13h (320x200x256)
    static const uint8_t crtc_regs[] = {
        0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,  // 0x00-0x07
        0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 0x08-0x0F
        0x9C, 0x8E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,  // 0x10-0x17
        0xFF                                             // 0x18
    };

    for (uint8_t i = 0; i < sizeof(crtc_regs); i++) {
        out(CRT_ADDR_REG, i);
        out(CRT_DATA_REG, crtc_regs[i]);
    }

    // Graphics Controller registers
    out(GRAP_ADDR_REG, 0x00); out(GRAP_DATA_REG, 0x00);  // Set/Reset
    out(GRAP_ADDR_REG, 0x01); out(GRAP_DATA_REG, 0x00);  // Enable Set/Reset
    out(GRAP_ADDR_REG, 0x02); out(GRAP_DATA_REG, 0x00);  // Color Compare
    out(GRAP_ADDR_REG, 0x03); out(GRAP_DATA_REG, 0x00);  // Data Rotate
    out(GRAP_ADDR_REG, 0x04); out(GRAP_DATA_REG, 0x00);  // Read Map Select
    out(GRAP_ADDR_REG, 0x05); out(GRAP_DATA_REG, 0x40);  // Mode
    out(GRAP_ADDR_REG, 0x06); out(GRAP_DATA_REG, 0x05);  // Miscellaneous
    out(GRAP_ADDR_REG, 0x07); out(GRAP_DATA_REG, 0x0F);  // Color Don't Care
    out(GRAP_ADDR_REG, 0x08); out(GRAP_DATA_REG, 0xFF);  // Bit Mask

    // Attribute Controller registers
    in(INPUT_STATUS_1);  // Reset attribute controller flip-flop
    
    // Palette registers (0-15)
    for (uint8_t i = 0; i < 0x10; i++) {
        out(ATTR_ADDR_REG, i);
        out(ATTR_DATA_REG, i);
    }
    
    // Attribute controller mode registers
    out(ATTR_ADDR_REG, 0x10); out(ATTR_DATA_REG, 0x41);  // Mode Control
    out(ATTR_ADDR_REG, 0x11); out(ATTR_DATA_REG, 0x00);  // Overscan Color
    out(ATTR_ADDR_REG, 0x12); out(ATTR_DATA_REG, 0x0F);  // Color Plane Enable
    out(ATTR_ADDR_REG, 0x13); out(ATTR_DATA_REG, 0x00);  // Horizontal Pixel Panning
    out(ATTR_ADDR_REG, 0x14); out(ATTR_DATA_REG, 0x00);  // Color Select

    // Enable video (important!)
    in(INPUT_STATUS_1);  // Reset flip-flop
    out(ATTR_ADDR_REG, 0x20);  // Bit 5 enables video

    // Set up the standard VGA 256-color palette for mode 13h
    // The standard VGA palette uses 6-bits per RGB component (0-63)
    out(DAC_WRITE_ADDR, 0);

    // First 16 colors - EGA compatibility
    static const uint8_t standard_palette[16][3] = {
        {0, 0, 0},       // 0: Black
        {0, 0, 42},      // 1: Blue
        {0, 42, 0},      // 2: Green
        {0, 42, 42},     // 3: Cyan
        {42, 0, 0},      // 4: Red
        {42, 0, 42},     // 5: Magenta
        {42, 21, 0},     // 6: Brown
        {42, 42, 42},    // 7: Light Gray
        {21, 21, 21},    // 8: Dark Gray
        {21, 21, 63},    // 9: Light Blue
        {21, 63, 21},    // 10: Light Green
        {21, 63, 63},    // 11: Light Cyan
        {63, 21, 21},    // 12: Light Red
        {63, 21, 63},    // 13: Light Magenta
        {63, 63, 21},    // 14: Yellow
        {63, 63, 63},    // 15: White
    };

    // Load the first 16 colors (EGA compatibility colors)
    for (int i = 0; i < 16; i++) {
        out(DAC_DATA_REG, standard_palette[i][0]);
        out(DAC_DATA_REG, standard_palette[i][1]);
        out(DAC_DATA_REG, standard_palette[i][2]);
    }

    // Colors 16-231: 6x6x6 color cube (216 colors)
    for (int r = 0; r < 6; r++) {
        for (int g = 0; g < 6; g++) {
            for (int b = 0; b < 6; b++) {
                uint8_t red = r * 63 / 5;
                uint8_t green = g * 63 / 5;
                uint8_t blue = b * 63 / 5;
                
                out(DAC_DATA_REG, red);
                out(DAC_DATA_REG, green);
                out(DAC_DATA_REG, blue);
            }
        }
    }

    // Colors 232-255: Grayscale ramp (24 shades)
    for (int i = 1; i < 24; i++) {
        uint8_t gray = i * 63 / 23;
        out(DAC_DATA_REG, gray);
        out(DAC_DATA_REG, gray);
        out(DAC_DATA_REG, gray);
    }
}

void graphics_exit(void) {
    // Switch back to text mode 80x25
    __asm__ volatile(
        "movb $0x03, %%al\n"
        "movb $0x00, %%ah\n"
        "int $0x10\n"
        :
        :
        : "eax"
    );
}

void graphics_pixel(uint16_t x, uint16_t y, uint8_t color) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) {
        return;
    }
    VGA_MEMORY[y * VGA_WIDTH + x] = color;
}

void graphics_clear(uint8_t color) {
    for (uint32_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA_MEMORY[i] = color;
    }
}

void graphics_char(uint16_t x, uint16_t y, unsigned char c, uint8_t color, uint8_t bgcolor) {
    if (c >= 128) return;

    const uint8_t* char_data = lookup[(uint8_t)c];
    uint8_t size = char_data[0];

    // Clear background: 5 columns x 8 rows (matching reference)
    // Note: Transparent mode (bgcolor=0xFF) is handled by kernel syscall handler
    // which calls graphics_char_transparent() instead
    for (uint8_t row = 0; row < 8; row++) {
        for (uint8_t col = 0; col < 5; col++) {
            graphics_pixel(x + col, y + row, bgcolor);
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

void graphics_string(uint16_t x, uint16_t y, const char* str, uint8_t color, uint8_t bgcolor) {
    uint16_t current_x = x;
    uint16_t current_y = y;
    
    for (size_t i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\n') {
            current_x = x;
            current_y += 8;
            continue;
        }
        graphics_char(current_x, current_y, str[i], color, bgcolor);
        current_x += 5;  // 5 pixels char width (matching reference)
        if (current_x >= VGA_WIDTH - 5) {
            current_x = x;
            current_y += 8;
        }
    }
}

void graphics_set_cursor(uint16_t x, uint16_t y) {
    cursor_x = x;
    cursor_y = y;
}

void graphics_get_cursor(uint16_t *x, uint16_t *y) {
    if (x) *x = cursor_x;
    if (y) *y = cursor_y;
}

void graphics_scroll_cursor(int16_t dx, int16_t dy) {
    int32_t new_x = (int32_t)cursor_x + dx;
    int32_t new_y = (int32_t)cursor_y + dy;

    // Clamp to screen bounds
    if (new_x < 0) new_x = 0;
    if (new_x >= VGA_WIDTH) new_x = VGA_WIDTH - 8;
    if (new_y < 0) new_y = 0;
    if (new_y >= VGA_HEIGHT) new_y = VGA_HEIGHT - 8;

    cursor_x = (uint16_t)new_x;
    cursor_y = (uint16_t)new_y;
}

void graphics_move_cursor(int16_t dx, int16_t dy) {
    graphics_scroll_cursor(dx, dy);
}

void graphics_rect_fill(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t color) {
    for (uint16_t dy = 0; dy < height; dy++) {
        for (uint16_t dx = 0; dx < width; dx++) {
            uint16_t px = x + dx;
            uint16_t py = y + dy;
            if (px < VGA_WIDTH && py < VGA_HEIGHT) {
                graphics_pixel(px, py, color);
            }
        }
    }
}

void graphics_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t color) {
    // Top and bottom edges
    for (uint16_t dx = 0; dx < width; dx++) {
        graphics_pixel(x + dx, y, color);
        graphics_pixel(x + dx, y + height - 1, color);
    }
    
    // Left and right edges
    for (uint16_t dy = 0; dy < height; dy++) {
        graphics_pixel(x, y + dy, color);
        graphics_pixel(x + width - 1, y + dy, color);
    }
}

void graphics_fill_screen_with_color() {
    for (uint16_t y = 0; y < VGA_HEIGHT; y++) {
        for (uint16_t x = 0; x < VGA_WIDTH; x++) {
            uint8_t color = ((x / 10) + (y / 10)) % 256;
            graphics_pixel(x, y, color);
        }
    }
}

void graphics_draw_cursor(void) {
    if (!cursor_visible) {
        graphics_char(cursor_x, cursor_y, cursor_char, cursor_fg_color, cursor_bg_color);
        cursor_visible = true;
    }
}

void graphics_erase_cursor(void) {
    if (cursor_visible) {
        graphics_rect_fill(cursor_x, cursor_y, 8, 8, cursor_bg_color);
        cursor_visible = false;
    }
}

void graphics_store_char_at_cursor(char c) {
    cursor_char = c;
}

void graphics_set_cursor_colors(uint8_t fg_color, uint8_t bg_color) {
    cursor_fg_color = fg_color;
    cursor_bg_color = bg_color;
}

void graphics_blink_cursor(void) {
    if (cursor_visible) {
        graphics_erase_cursor();
    } else {
        graphics_draw_cursor();
    }
}

void graphics_scroll(uint16_t lines, uint8_t color) {
    if (lines >= VGA_HEIGHT) {
        graphics_clear(color);
        return;
    }

    // Reserve last row for clock (8 pixels high)
    uint16_t scrollable_height = VGA_HEIGHT - 8;  // 200 - 8 = 192 pixels
    
    if (lines >= scrollable_height) {
        // Clear only scrollable area, preserve clock row
        for (uint32_t i = 0; i < VGA_WIDTH * scrollable_height; i++) {
            VGA_MEMORY[i] = color;
        }
        return;
    }

    // Move existing content up (only in scrollable area)
    uint32_t bytes_to_move = VGA_WIDTH * (scrollable_height - lines);
    for (uint32_t i = 0; i < bytes_to_move; i++) {
        VGA_MEMORY[i] = VGA_MEMORY[i + (lines * VGA_WIDTH)];
    }

    // Fill the newly exposed area with specified color (between scrolled content and clock)
    uint32_t start_offset = bytes_to_move;
    uint32_t end_offset = VGA_WIDTH * scrollable_height;
    for (uint32_t i = start_offset; i < end_offset; i++) {
        VGA_MEMORY[i] = color;
    }
}

void graphics_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color) {
    // Bresenham's line algorithm
    int16_t dx = x2 > x1 ? x2 - x1 : x1 - x2;
    int16_t dy = y2 > y1 ? y2 - y1 : y1 - y2;
    int16_t sx = x1 < x2 ? 1 : -1;
    int16_t sy = y1 < y2 ? 1 : -1;
    int16_t err = dx - dy;

    while (1) {
        graphics_pixel(x1, y1, color);

        if (x1 == x2 && y1 == y2) {
            break;
        }

        int16_t e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void graphics_circle_filled(uint16_t center_x, uint16_t center_y, uint16_t radius, uint8_t color) {
    // Midpoint circle algorithm with fill
    int16_t x = radius;
    int16_t y = 0;
    int16_t err = 0;

    while (x >= y) {
        // Draw horizontal lines for filled circle
        for (int16_t i = center_x - x; i <= center_x + x; i++) {
            graphics_pixel(i, center_y + y, color);
            graphics_pixel(i, center_y - y, color);
        }
        for (int16_t i = center_x - y; i <= center_x + y; i++) {
            graphics_pixel(i, center_y + x, color);
            graphics_pixel(i, center_y - x, color);
        }

        y++;
        err += 1 + 2 * y;
        if (2 * (err - x) + 1 > 0) {
            x--;
            err += 1 - 2 * x;
        }
    }
}

// Wallpaper functions are in wallpaper.c (kernel only)
// For user programs, these are provided via syscalls
