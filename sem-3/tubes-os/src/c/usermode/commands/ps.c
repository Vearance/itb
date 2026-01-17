#include "header/usermode/commands/ps.h"
#include "header/interrupt/interrupt.h"
#include "header/process/process.h"
#include <stdint.h>

// External print functions from user-shell
extern void print_string(const char *str);
extern void print_string_color(const char *str, uint8_t color);
extern void flush_output_buffer(void);

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

// Helper to get state name
static const char* get_state_name(PROCESS_STATE state)
{
    switch (state)
    {
        case PROCESS_STATE_INACTIVE: return "INACTIVE";
        case PROCESS_STATE_READY:    return "READY";
        case PROCESS_STATE_RUNNING:  return "RUNNING";
        case PROCESS_STATE_WAITING:  return "WAITING";
        default:                     return "UNKNOWN";
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

int8_t ps_command(void)
{
    // Print header
    print_string_color("PID   STATE      NAME\n", 0x0E);  // Yellow
    print_string_color("---   -----      ----\n", 0x08);  // Dark gray
    
    // Iterate through all process slots by index
    ProcessMetadata metadata;
    int found = 0;
    
    for (uint32_t i = 0; i < PROCESS_COUNT_MAX; i++)
    {
        // Get process info by index (not PID)
        syscall(SYS_GET_PROCESS_BY_INDEX, i, (uint32_t)&metadata, 0);
        
        // Check if process is active
        if (metadata.state != PROCESS_STATE_INACTIVE)
        {
            // Print PID (padded to 6 chars)
            print_number(metadata.pid);
            if (metadata.pid < 10) print_string("     ");
            else if (metadata.pid < 100) print_string("    ");
            else print_string("   ");
            
            // Print state (padded to 11 chars)
            const char *state = get_state_name(metadata.state);
            print_string(state);
            
            // Padding based on state length
            uint32_t state_len = 0;
            for (const char *s = state; *s; s++) state_len++;
            for (uint32_t i = state_len; i < 11; i++) print_string(" ");
            
            // Print name
            if (metadata.name[0] != '\0')
                print_string(metadata.name);
            else
                print_string("<unnamed>");
            print_string("\n");
            
            found++;
        }
    }
    
    if (found == 0)
    {
        print_string("No active processes\n");
    }
    else
    {
        print_string("\nTotal: ");
        print_number(found);
        print_string(" process(es)\n");
    }
    
    flush_output_buffer();
    return 0;
}
