#include "header/usermode/commands/kill.h"
#include "header/interrupt/interrupt.h"
#include <stdint.h>

// External print functions from user-shell
extern void print_string(const char *str);
extern void print_string_color(const char *str, uint8_t color);
extern void flush_output_buffer(void);
extern void refresh_screen(void);

// Helper to print number
static void print_number(uint32_t num)
{
    if (num == 0)
    {
        print_string("0");
        return;
    }
    
    char buf[12];
    int i = 0;
    while (num > 0)
    {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    // Reverse and print
    while (i > 0)
    {
        char c[2] = {buf[--i], '\0'};
        print_string(c);
    }
}

// Syscall wrapper
static int32_t syscall(uint32_t num, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    int32_t ret;
    __asm__ volatile(
        "int $0x30"
        : "=a"(ret)
        : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3)
        : "memory");
    return ret;
}

int8_t kill_command(uint32_t pid)
{
    // Try to kill the process
    int32_t result = -1;
    syscall(SYS_KILL_PROCESS, pid, (uint32_t)&result, 0);
    
    if (result == 0)
    {
        // Success - redraw screen to clear any graphics left by killed process
        refresh_screen();
        
        print_string_color("Killed process ", 0x0A);  // Green
        print_number(pid);
        print_string("\n");
        flush_output_buffer();
        return 0;
    }
    else if (result == 2)
    {
        print_string_color("kill: cannot kill process ", 0x0C);
        print_number(pid);
        print_string_color(" (currently running)\n", 0x0C);
        flush_output_buffer();
        return -1;
    }
    else
    {
        print_string_color("kill: process ", 0x0C);
        print_number(pid);
        print_string_color(" not found\n", 0x0C);
        flush_output_buffer();
        return -1;
    }
}
