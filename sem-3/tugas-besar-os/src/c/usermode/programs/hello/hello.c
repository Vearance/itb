#include <stdint.h>
#include "header/interrupt/interrupt.h"
#include "header/text/framebuffer.h"
#include "header/graphics/graphics.h"

// Minimal syscall wrapper used by user programs
static void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx)
{
    __asm__ volatile("mov %0, %%ebx" : : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : : "r"(eax));
    __asm__ volatile("int $0x30");
}

static void puts_at(uint32_t row, uint32_t col, const char *text, uint8_t fg, uint8_t bg)
{
    uint32_t colors = ((uint32_t)bg << 8) | fg;
    syscall(SYS_GFX_PUTS, col | (row << 16), (uint32_t)text, colors);
}

int main(void)
{
    // Clear framebuffer and print a friendly message
    syscall(SYS_GFX_CLEAR, 0, 0, 0);
    puts_at(0, 0, "hello world\n", COLOR_WHITE, COLOR_BLACK);
    return 0;
}
