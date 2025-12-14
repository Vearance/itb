#include "header/driver/cmos.h"
#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/usermode/user-shell.h"
#include "header/interrupt/interrupt.h"

// Graphics mode settings
#define GFX_WIDTH 320
#define GFX_HEIGHT 200
#define GFX_CHAR_WIDTH 5   // 5 pixels wide (matching graphics_reference.c)
#define GFX_CHAR_HEIGHT 8

// Special color for transparent background (uses wallpaper)
#define COLOR_TRANSPARENT 0xFF

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

static void gfx_putchar(uint16_t x, uint16_t y, char c, uint8_t fg, uint8_t bg)
{
    uint32_t pos = (uint32_t)x | ((uint32_t)y << 16);
    uint32_t char_info = (uint32_t)c | ((uint32_t)fg << 8) | ((uint32_t)bg << 16);
    syscall(SYS_GFX_PUTCHAR, pos, char_info, 0);
}

// Print string character by character to ensure proper rendering
static void gfx_puts_clear(uint16_t x, uint16_t y, const char *str, uint8_t fg, uint8_t bg)
{
    uint16_t current_x = x;
    while (*str)
    {
        gfx_putchar(current_x, y, *str, fg, bg);
        current_x += GFX_CHAR_WIDTH;
        str++;
    }
}

int main(void)
{
    struct Time current_time;
    struct Time past_time;

    memset(&past_time, 0, sizeof(struct Time));
    past_time.second = 0xFF; // Force initial update

    while (true)
    {
        memset(&current_time, 0, sizeof(struct Time));

        syscall(SYS_TIME_GET, (uint32_t)&current_time, 0, 0);

        // Only update display if time has changed
        if (current_time.second != past_time.second ||
            current_time.minute != past_time.minute ||
            current_time.hour != past_time.hour)
        {
            // Calculate adjusted hour (UTC+7)
            uint8_t adjusted_hour = (current_time.hour + 7) % 24;
            
            char time_str[9]; // HH:MM:SS + null terminator
            time_str[0] = (adjusted_hour / 10) + '0';
            time_str[1] = (adjusted_hour % 10) + '0';
            time_str[2] = ':';
            time_str[3] = (current_time.minute / 10) + '0';
            time_str[4] = (current_time.minute % 10) + '0';
            time_str[5] = ':';
            time_str[6] = (current_time.second / 10) + '0';
            time_str[7] = (current_time.second % 10) + '0';
            time_str[8] = '\0';

            // Print time at bottom right corner using graphics mode
            // 8 chars * 5 pixels = 40 pixels width
            uint16_t x = GFX_WIDTH - (8 * GFX_CHAR_WIDTH);  // 320 - 40 = 280
            uint16_t y = GFX_HEIGHT - GFX_CHAR_HEIGHT;       // 200 - 8 = 192
            
            // Use character-by-character rendering with transparent background
            gfx_puts_clear(x, y, time_str, 0x0E, COLOR_TRANSPARENT);  // Yellow on transparent

            // Update past time
            past_time.second = current_time.second;
            past_time.minute = current_time.minute;
            past_time.hour = current_time.hour;
        }

        // Small delay to prevent busy-waiting consuming too much CPU
        // Adjust this value for smoother updates
        for (volatile uint32_t i = 0; i < 200000; i++)
            ;
    }

    return 0;
}