#include "header/cpu/gdt.h"
#include "header/cpu/portio.h"
#include "header/driver/ext2.h"
#include "header/driver/keyboard.h"
#include "header/driver/cmos.h"
#include "header/interrupt/interrupt.h"
#include "header/text/framebuffer.h"
#include "header/graphics/graphics.h"
#include "header/graphics/wallpaper.h"
#include "header/graphics/badapple.h"
#include "header/process/process.h"
#include "header/process/scheduler.h"
#include "header/memory/heap.h"
#include "header/driver/audio.h"

void io_wait(void)
{
    out(0x80, 0);
}

void pic_ack(uint8_t irq)
{
    if (irq >= 8)
        out(PIC2_COMMAND, PIC_ACK);
    out(PIC1_COMMAND, PIC_ACK);
}

void pic_remap(void)
{
    out(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    out(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    out(PIC1_DATA, PIC1_OFFSET); // ICW2: Master PIC vector offset
    io_wait();
    out(PIC2_DATA, PIC2_OFFSET); // ICW2: Slave PIC vector offset
    io_wait();
    out(PIC1_DATA, 0b0100); // ICW3: tell Master PIC, slave PIC at IRQ2 (0000 0100)
    io_wait();
    out(PIC2_DATA, 0b0010); // ICW3: tell Slave PIC its cascade identity (0000 0010)
    io_wait();

    out(PIC1_DATA, ICW4_8086);
    io_wait();
    out(PIC2_DATA, ICW4_8086);
    io_wait();

    // Disable all interrupts
    out(PIC1_DATA, PIC_DISABLE_ALL_MASK);
    out(PIC2_DATA, PIC_DISABLE_ALL_MASK);
}

void main_interrupt_handler(struct InterruptFrame frame)
{
    switch (frame.int_number)
    {
    case 14:
        // Page Fault - display error info and halt
        {
            uint32_t fault_addr;
            __asm__ volatile("mov %%cr2, %0" : "=r"(fault_addr));
            
            // Write "PF:" to top-left of screen
            framebuffer_write(0, 0, 'P', 0x4F, 0x00);
            framebuffer_write(0, 1, 'F', 0x4F, 0x00);
            framebuffer_write(0, 2, ':', 0x4F, 0x00);
            
            // Display fault address in hex
            for (int i = 0; i < 8; i++) {
                uint8_t nibble = (fault_addr >> (28 - i * 4)) & 0xF;
                char c = nibble < 10 ? '0' + nibble : 'A' + nibble - 10;
                framebuffer_write(0, 4 + i, c, 0x4F, 0x00);
            }
            
            // Display EIP
            framebuffer_write(0, 14, 'E', 0x4F, 0x00);
            framebuffer_write(0, 15, 'I', 0x4F, 0x00);
            framebuffer_write(0, 16, 'P', 0x4F, 0x00);
            framebuffer_write(0, 17, ':', 0x4F, 0x00);
            for (int i = 0; i < 8; i++) {
                uint8_t nibble = (frame.int_stack.eip >> (28 - i * 4)) & 0xF;
                char c = nibble < 10 ? '0' + nibble : 'A' + nibble - 10;
                framebuffer_write(0, 19 + i, c, 0x4F, 0x00);
            }
        }
        __asm__("hlt");
        break;
    case IRQ_TIMER + PIC1_OFFSET:
        pic_ack(IRQ_TIMER);
        scheduler_handle_timer_interrupt(frame);
        break;
    case IRQ_KEYBOARD + PIC1_OFFSET:
        keyboard_isr();
        break;
    case 0x30: // Syscall
        syscall_handler(frame);
        break;
    }
}

void syscall_handler(struct InterruptFrame frame)
{
    uint32_t row, col;
    uint8_t fg, bg;

    switch (frame.cpu.general.eax)
    {
    case SYS_READ:
        *((int8_t *)frame.cpu.general.ecx) = read(
            *(struct EXT2DriverRequest *)frame.cpu.general.ebx);
        break;

    case SYS_READ_DIR:
        *((int8_t *)frame.cpu.general.ecx) = read_directory(
            (struct EXT2DriverRequest *)frame.cpu.general.ebx);
        break;
    case SYS_WRITE:
        *((int8_t *)frame.cpu.general.ecx) = write(
            (struct EXT2DriverRequest *)frame.cpu.general.ebx);
        break;
    case SYS_DELETE:
        *((int8_t *)frame.cpu.general.ecx) = delete(
            *(struct EXT2DriverRequest *)frame.cpu.general.ebx);
        break;
    case SYS_GETCHAR:
    {
        char *char_ptr = (char *)frame.cpu.general.ebx;
        get_keyboard_buffer(char_ptr);
    }
    break;
    case SYS_PUTCHAR:
        row = ((CursorPosition *)frame.cpu.general.edx)->row;
        col = ((CursorPosition *)frame.cpu.general.edx)->col;
        fg = (uint8_t)((frame.cpu.general.ecx) & 0xFF);
        bg = (uint8_t)((frame.cpu.general.ecx >> 8) & 0xFF);
        framebuffer_write(
            row,
            col,
            (char)(frame.cpu.general.ebx & 0xFF),
            fg,
            bg);
        break;
    case SYS_PUTS:
        row = ((CursorPosition *)frame.cpu.general.edx)->row;
        col = ((CursorPosition *)frame.cpu.general.edx)->col;
        fg = (uint8_t)((frame.cpu.general.ecx) & 0xFF);
        bg = (uint8_t)((frame.cpu.general.ecx >> 8) & 0xFF);
        {
            char *str = (char *)frame.cpu.general.ebx;
            while (*str)
            {
                // Only write valid printable characters and newline
                if ((*str >= 32 && *str <= 126) || *str == '\n')
                {
                    framebuffer_write(
                        row,
                        col,
                        *str,
                        fg,
                        bg);
                    col++;
                    if (col >= FRAMEBUFFER_WIDTH || *str == '\n')
                    {
                        col = 0;
                        row++;
                    }
                }
                str++;
            }
        }
        break;
    case SYS_ACT_KEYBOARD:
        keyboard_state_activate();
        activate_keyboard_interrupt();
        break;
    case SYS_SET_CURSOR:
        row = ((CursorPosition *)frame.cpu.general.ebx)->row;
        col = ((CursorPosition *)frame.cpu.general.ebx)->col;
        framebuffer_set_cursor(row, col);
        break;
    case SYS_CLEAR_FRAMEBUFFER:
        framebuffer_clear();
        framebuffer_set_cursor(0, 0);
        break;
    case SYS_SCROLL:
    {
        // Scroll framebuffer up by one line
        // Shift all rows up by one, then clear the last row
        uint8_t *fb_ptr = (uint8_t *)FRAMEBUFFER_MEMORY_OFFSET;

        // Copy each row to the row above it (skip first row)
        for (uint32_t row = 0; row < FRAMEBUFFER_HEIGHT - 1; row++)
        {
            for (uint32_t col = 0; col < FRAMEBUFFER_WIDTH; col++)
            {
                uint32_t src_offset = (row + 1) * FRAMEBUFFER_WIDTH * 2 + col * 2;
                uint32_t dst_offset = row * FRAMEBUFFER_WIDTH * 2 + col * 2;
                fb_ptr[dst_offset] = fb_ptr[src_offset];         // Character
                fb_ptr[dst_offset + 1] = fb_ptr[src_offset + 1]; // Color
            }
        }

        // Clear the last row
        uint32_t last_row = FRAMEBUFFER_HEIGHT - 1;
        for (uint32_t col = 0; col < FRAMEBUFFER_WIDTH; col++)
        {
            uint32_t offset = last_row * FRAMEBUFFER_WIDTH * 2 + col * 2;
            fb_ptr[offset] = ' ';      // Empty character
            fb_ptr[offset + 1] = 0x07; // Gray on black
        }
    }
    break;
    case SYS_RENAME:
        // Rename a file or directory
        // ebx = pointer to EXT2DriverRequest
        // ecx = pointer to return code
        // The request should have:
        // - name: old name (current name)
        // - parent_inode: parent directory inode
        // - buf: pointer to new name string
        // - name_len: length of new name
        {
            struct EXT2DriverRequest *req = (struct EXT2DriverRequest *)frame.cpu.general.ebx;
            int8_t retval = rename_entry(req);
            *((int8_t *)frame.cpu.general.ecx) = retval;
        }
        break;
    case SYS_CREATE_PROCESS:
        // Create a new user process
        process_exec(
            (char *)frame.cpu.general.ebx,
            (uint32_t)frame.cpu.general.ecx,
            (int32_t *)frame.cpu.general.edx);
        break;
    case SYS_GET_PROCESS_INFO:
        process_info(
            (uint32_t)frame.cpu.general.ebx,
            (ProcessMetadata *)frame.cpu.general.ecx);
        break;
    case SYS_TERMINATE_PROCESS:
    {
        struct ProcessControlBlock *current_pcb = process_get_current_running_pcb_pointer();
        process_destroy(current_pcb->metadata.pid);
    }
    break;
    case SYS_KILL_PROCESS:
    {
        // Kill process by PID (ebx = pid, ecx = pointer to result)
        // Result: 0 on success, 1 if not found, 2 if running
        uint32_t pid_to_kill = (uint32_t)frame.cpu.general.ebx;
        int32_t *result_ptr = (int32_t *)frame.cpu.general.ecx;
        
        // First find the process to check its state
        bool found = false;
        for (int32_t i = 0; i < PROCESS_COUNT_MAX; i++)
        {
            struct ProcessControlBlock *pcb = &(_process_list[i]);
            if (pcb->metadata.state != PROCESS_STATE_INACTIVE && pcb->metadata.pid == pid_to_kill)
            {
                found = true;
                if (pcb->metadata.state == PROCESS_STATE_RUNNING)
                {
                    *result_ptr = 2; // Cannot kill running process
                }
                else
                {
                    // Process found and not running - destroy it
                    process_destroy(pid_to_kill);
                    *result_ptr = 0; // Success
                }
                break;
            }
        }
        if (!found)
        {
            *result_ptr = 1; // Not found
        }
    }
    break;
    case SYS_GET_PROCESS_COUNT:
    {
        // Return active process count via pointer (ebx = pointer to result)
        uint32_t *count_ptr = (uint32_t *)frame.cpu.general.ebx;
        if (count_ptr) {
            *count_ptr = process_manager.active_process_count;
        }
    }
    break;
    case SYS_GET_PROCESS_BY_INDEX:
    {
        // Get process info by index in process list (ebx = index, ecx = pointer to metadata)
        uint32_t index = (uint32_t)frame.cpu.general.ebx;
        ProcessMetadata *meta = (ProcessMetadata *)frame.cpu.general.ecx;
        if (index < PROCESS_COUNT_MAX) {
            struct ProcessControlBlock *pcb = &(_process_list[index]);
            meta->pid = pcb->metadata.pid;
            meta->state = pcb->metadata.state;
            memcpy(meta->name, pcb->metadata.name, PROCESS_NAME_LENGTH_MAX);
        } else {
            meta->state = PROCESS_STATE_INACTIVE;
        }
    }
    break;
    case SYS_TIME_GET:
    {
        struct
        {
            uint8_t seconds;
            uint8_t minutes;
            uint8_t hours;
            uint8_t day;
            uint8_t month;
            uint16_t year;
            uint8_t century;
        } *time_info = (typeof(time_info))frame.cpu.general.ebx;

        struct Time time;
        read_rtc(&time);

        time_info->seconds = time.second;
        time_info->minutes = time.minute;
        time_info->hours = time.hour;
        time_info->day = time.day;
        time_info->month = time.month;
        time_info->year = time.year;
        time_info->century = time.century;
        break;
    }
    case SYS_GFX_INIT:
        // Initialize graphics mode (320x200x256)
        graphics_initialize();
        graphics_clear(COLOR_BLACK);
        break;
    case SYS_GFX_PUTCHAR:
    {
        // Draw character with custom font
        // ebx = packed (x | (y << 16))
        // ecx = packed (char | (fg_color << 8) | (bg_color << 16))
        uint16_t x = (uint16_t)(frame.cpu.general.ebx & 0xFFFF);
        uint16_t y = (uint16_t)((frame.cpu.general.ebx >> 16) & 0xFFFF);
        unsigned char c = (unsigned char)(frame.cpu.general.ecx & 0xFF);
        uint8_t fg_color = (uint8_t)((frame.cpu.general.ecx >> 8) & 0xFF);
        uint8_t bg_color = (uint8_t)((frame.cpu.general.ecx >> 16) & 0xFF);
        
        // Use transparent mode if bg_color is 0xFF
        if (bg_color == 0xFF) {
            graphics_char_transparent(x, y, c, fg_color);
        } else {
            graphics_char(x, y, c, fg_color, bg_color);
        }
        break;
    }
    case SYS_GFX_PUTS:
    {
        // Draw string with custom font
        // ebx = packed (x | (y << 16))
        // ecx = pointer to string
        // edx = packed (fg_color | (bg_color << 8))
        uint16_t x = (uint16_t)(frame.cpu.general.ebx & 0xFFFF);
        uint16_t y = (uint16_t)((frame.cpu.general.ebx >> 16) & 0xFFFF);
        char *str = (char *)frame.cpu.general.ecx;
        uint8_t fg_color = (uint8_t)(frame.cpu.general.edx & 0xFF);
        uint8_t bg_color = (uint8_t)((frame.cpu.general.edx >> 8) & 0xFF);
        
        // Use transparent mode if bg_color is 0xFF
        if (bg_color == 0xFF) {
            graphics_string_transparent(x, y, str, fg_color);
        } else {
            graphics_string(x, y, str, fg_color, bg_color);
        }
        break;
    }
    case SYS_GFX_CLEAR:
        // Clear graphics screen with color
        // ebx = color
        graphics_clear((uint8_t)(frame.cpu.general.ebx & 0xFF));
        break;
    case SYS_GFX_PIXEL:
    {
        // Draw single pixel
        // ebx = packed (x | (y << 16))
        // ecx = color
        uint16_t x = (uint16_t)(frame.cpu.general.ebx & 0xFFFF);
        uint16_t y = (uint16_t)((frame.cpu.general.ebx >> 16) & 0xFFFF);
        uint8_t color = (uint8_t)(frame.cpu.general.ecx & 0xFF);
        graphics_pixel(x, y, color);
        break;
    }
    case SYS_GFX_SCROLL:
    {
        // Scroll graphics screen
        // ebx = lines to scroll
        // ecx = fill color (0xFF = transparent/wallpaper)
        uint16_t lines = (uint16_t)(frame.cpu.general.ebx & 0xFFFF);
        uint8_t color = (uint8_t)(frame.cpu.general.ecx & 0xFF);
        
        if (color == 0xFF) {
            graphics_scroll_transparent(lines);
        } else {
            graphics_scroll(lines, color);
        }
        break;
    }
    case SYS_GFX_DRAW_WALLPAPER:
        // Draw wallpaper to fill screen
        graphics_draw_wallpaper();
        break;
    case SYS_GFX_RESTORE_BG:
    {
        // Restore wallpaper region
        // ebx = packed (x | (y << 16))
        // ecx = packed (width | (height << 16))
        uint16_t x = (uint16_t)(frame.cpu.general.ebx & 0xFFFF);
        uint16_t y = (uint16_t)((frame.cpu.general.ebx >> 16) & 0xFFFF);
        uint16_t width = (uint16_t)(frame.cpu.general.ecx & 0xFFFF);
        uint16_t height = (uint16_t)((frame.cpu.general.ecx >> 16) & 0xFFFF);
        graphics_draw_wallpaper_region(x, y, width, height);
        break;
    }
    case SYS_MALLOC:
    {
        // Allocate memory from kernel heap
        // ebx = size to allocate
        // ecx = pointer to store result pointer
        uint32_t size = frame.cpu.general.ebx;
        void **result_ptr = (void **)frame.cpu.general.ecx;
        if (result_ptr) {
            *result_ptr = kmalloc(size);
        }
        break;
    }
    case SYS_FREE:
    {
        // Free memory from kernel heap
        // ebx = pointer to free
        void *ptr = (void *)frame.cpu.general.ebx;
        kfree(ptr);
        break;
    }
    case SYS_REALLOC:
    {
        // Reallocate memory
        // ebx = original pointer
        // ecx = new size
        // edx = pointer to store result pointer
        void *ptr = (void *)frame.cpu.general.ebx;
        uint32_t size = frame.cpu.general.ecx;
        void **result_ptr = (void **)frame.cpu.general.edx;
        if (result_ptr) {
            *result_ptr = krealloc(ptr, size);
        }
        break;
    }
    case SYS_HEAP_STATS:
    {
        // Get heap statistics
        // ebx = pointer to HeapStats structure
        HeapStats *stats = (HeapStats *)frame.cpu.general.ebx;
        if (stats) {
            heap_get_stats(stats);
        }
        break;
    }
    case SYS_HEAP_TEST:
    {
        // Run heap test in kernel mode and return results
        // ebx = pointer to result structure
        struct {
            bool test1_alloc;
            bool test2_alloc;
            bool test3_calloc;
            bool test3_zeroed;
            bool test4_free;
            bool test5_realloc;
            bool test6_cleanup;
            uint32_t alloc_count;
            uint32_t free_count;
        } *result = (void *)frame.cpu.general.ebx;
        
        if (result) {
            // Initialize results
            result->test1_alloc = false;
            result->test2_alloc = false;
            result->test3_calloc = false;
            result->test3_zeroed = false;
            result->test4_free = false;
            result->test5_realloc = false;
            result->test6_cleanup = false;
            
            // Test 1: Allocate 64 bytes
            char *buf1 = (char *)kmalloc(64);
            if (buf1) {
                result->test1_alloc = true;
                // Write pattern
                for (int i = 0; i < 63; i++) buf1[i] = 'A' + (i % 26);
                buf1[63] = '\0';
            }
            
            // Test 2: Allocate 128 bytes
            char *buf2 = (char *)kmalloc(128);
            if (buf2) {
                result->test2_alloc = true;
                const char *msg = "Hello from kernel heap!";
                int i = 0;
                while (msg[i]) { buf2[i] = msg[i]; i++; }
                buf2[i] = '\0';
            }
            
            // Test 3: Allocate with zero (simulate calloc)
            uint8_t *buf3 = (uint8_t *)kzalloc(32);
            if (buf3) {
                result->test3_calloc = true;
                // Check if zeroed
                result->test3_zeroed = true;
                for (int i = 0; i < 32; i++) {
                    if (buf3[i] != 0) {
                        result->test3_zeroed = false;
                        break;
                    }
                }
            }
            
            // Test 4: Free first buffer
            if (buf1) {
                kfree(buf1);
                result->test4_free = true;
            }
            
            // Test 5: Allocate again (should reuse space)
            char *buf4 = (char *)kmalloc(32);
            if (buf4) {
                result->test5_realloc = true;
                const char *reuse_msg = "Reused!";
                int i = 0;
                while (reuse_msg[i]) { buf4[i] = reuse_msg[i]; i++; }
                buf4[i] = '\0';
            }
            
            // Test 6: Cleanup
            if (buf2) kfree(buf2);
            if (buf3) kfree(buf3);
            if (buf4) kfree(buf4);
            result->test6_cleanup = true;
            
            // Get stats
            HeapStats stats;
            heap_get_stats(&stats);
            result->alloc_count = stats.alloc_count;
            result->free_count = stats.free_count;
        }
        break;
    }
    case SYS_GFX_SET_WALLPAPER:
    {
        // Set current wallpaper by index
        // ebx = wallpaper index
        // ecx = pointer to result (0 = success, 1 = invalid index)
        uint32_t index = frame.cpu.general.ebx;
        uint32_t *result_ptr = (uint32_t *)frame.cpu.general.ecx;
        if (result_ptr) {
            *result_ptr = wallpaper_set_current(index) ? 0 : 1;
        }
        break;
    }
    case SYS_GFX_GET_WALLPAPER_INFO:
    {
        // Get wallpaper count and current index
        // ebx = pointer to count
        // ecx = pointer to current index
        uint32_t *count_ptr = (uint32_t *)frame.cpu.general.ebx;
        uint32_t *current_ptr = (uint32_t *)frame.cpu.general.ecx;
        if (count_ptr) {
            *count_ptr = wallpaper_get_count();
        }
        if (current_ptr) {
            *current_ptr = wallpaper_get_current();
        }
        break;
    }
    case SYS_GFX_GET_WALLPAPER_NAME:
    {
        // Get wallpaper name by index
        // ebx = wallpaper index
        // ecx = pointer to name buffer (should be at least 32 bytes)
        uint32_t index = frame.cpu.general.ebx;
        char *name_buf = (char *)frame.cpu.general.ecx;
        if (name_buf) {
            const char *name = wallpaper_get_name(index);
            // Copy name to buffer (max 31 chars + null)
            uint32_t i = 0;
            while (name[i] && i < 31) {
                name_buf[i] = name[i];
                i++;
            }
            name_buf[i] = '\0';
        }
        break;
    }
    case SYS_GFX_LOAD_WALLPAPER_FS:
    {
        // Load wallpaper bytes directly from filesystem
        // ebx = parent inode
        // ecx = pointer to filename (char*)
        // edx = pointer to result code (0 success)
        uint32_t parent_inode = frame.cpu.general.ebx;
        const char *filename = (const char *)frame.cpu.general.ecx;
        uint32_t *result_ptr = (uint32_t *)frame.cpu.general.edx;
        int result = wallpaper_load_from_fs(parent_inode, filename);
        if (result_ptr) {
            *result_ptr = (uint32_t)result;
        }
        break;
    }
    case SYS_BADAPPLE:
    {
        // Run Bad Apple animation in kernel mode
        // ebx = pointer to result (0 = success, -1 = error)
        int32_t *result_ptr = (int32_t *)frame.cpu.general.ebx;
        int result = badapple_run();
        if (result_ptr) {
            *result_ptr = result;
        }
        break;
    }
    case SYS_BEEP:
        audio_system_beep();
        break;
    default:
        break;
    }
}

void activate_keyboard_interrupt(void)
{
    out(PIC1_DATA, in(PIC1_DATA) & ~(1 << IRQ_KEYBOARD));
}

struct TSSEntry _interrupt_tss_entry = {
    .ss0 = GDT_KERNEL_DATA_SEGMENT_SELECTOR,
};

void set_tss_kernel_current_stack(void)
{
    uint32_t stack_ptr;
    // Reading base stack frame instead esp
    __asm__ volatile("mov %%ebp, %0" : "=r"(stack_ptr) : /* <Empty> */);
    // Add 8 because 4 for ret address and other 4 is for stack_ptr variable
    _interrupt_tss_entry.esp0 = stack_ptr + 8;
}
