#include <stdint.h>
#include <stdbool.h>
#include "header/cpu/gdt.h"
#include "header/text/framebuffer.h"
#include "header/graphics/graphics.h"
#include "header/graphics/wallpaper.h"
#include "header/kernel-entrypoint.h"
#include "header/interrupt/idt.h"
#include "header/interrupt/interrupt.h"
#include "header/driver/keyboard.h"
#include "header/driver/disk.h"
#include "header/driver/ext2.h"
#include "header/stdlib/string.h"
#include "header/memory/paging.h"
#include "header/memory/heap.h"
#include "header/process/process.h"
#include "header/process/scheduler.h"
#include "header/driver/audio.h"

// Helper variables for framebuffer
static uint8_t current_row = 0;
static uint8_t current_col = 0;

// Helper function to print string to framebuffer
void print(const char *str)
{
    while (*str)
    {
        if (*str == '\n')
        {
            current_row++;
            current_col = 0;
            if (current_row >= FRAMEBUFFER_HEIGHT)
            {
                current_row = 0;
            }
        }
        else
        {
            framebuffer_write(current_row, current_col, *str, 0x0F, 0x00);
            current_col++;
            if (current_col >= FRAMEBUFFER_WIDTH)
            {
                current_col = 0;
                current_row++;
                if (current_row >= FRAMEBUFFER_HEIGHT)
                {
                    current_row = 0;
                }
            }
        }
        str++;
    }
}

// Helper to print single char
void print_char(char c)
{
    if (c == '\n')
    {
        current_row++;
        current_col = 0;
    }
    else
    {
        framebuffer_write(current_row, current_col, c, 0x0F, 0x00);
        current_col++;
        if (current_col >= FRAMEBUFFER_WIDTH)
        {
            current_col = 0;
            current_row++;
        }
    }
}

void print_hex_byte(int8_t value)
{
    int8_t b = value; // keep only low 8 bits (so -1 -> 0xFF)
    int8_t hi = (b >> 4) & 0x0F;
    int8_t lo = b & 0x0F;
    char c;

    if (value == -1)
        print("=====================================================");

    c = (hi < 10) ? ('0' + hi) : ('A' + (hi - 10));
    print_char(c);
    c = (lo < 10) ? ('0' + lo) : ('A' + (lo - 10));
    print_char(c);
}

// Helper to print result status
void print_result(int8_t result, const char *test_name)
{
    print(test_name);
    print(": ");
    if (result == 0)
    {
        print("OK (code: 0x");
        print_hex_byte(result);
        print(")\n");
    }
    else
    {
        print("FAIL (code: 0x");
        print_hex_byte(result);
        print(")\n");
    }
}

// EXT2 CRUD Testing Function
// Tests basic CRUD operations and indirect block handling
// Test 1-9: Basic operations (create, read, update, delete)
// Test 10: Single indirect blocks (13 blocks = 12 direct + 1 indirect)
// Test 11: Boundary test (12 blocks = all direct, no indirect)
void test_ext2_crud(void)
{
    current_row = 0;
    current_col = 0;

    print("=== EXT2 CRUD Testing ===\n\n");

    // Test 1: Write a file to root directory
    print("Test 1: Create file 'test.txt'\n");
    struct EXT2DriverRequest write_req;
    char test_data[] = "Hello EXT2!";
    write_req.parent_inode = 1; // root directory
    write_req.name = "test.txt";
    write_req.name_len = 8;
    write_req.buf = test_data;
    write_req.buffer_size = 11;
    write_req.is_directory = false;

    int8_t result = write(&write_req);
    print_result(result, "  write()");

    // Test 2: Read the file back
    print("\nTest 2: Read file 'test.txt'\n");
    struct EXT2DriverRequest read_req;
    char read_buffer[512];
    memset(read_buffer, 0, 512);
    read_req.parent_inode = 1;
    read_req.name = "test.txt";
    read_req.name_len = 8;
    read_req.buf = read_buffer;
    read_req.buffer_size = 512;

    result = read(read_req);
    print_result(result, "  read()");

    // Verify data
    if (result == 0)
    {
        print("  Data: '");
        for (int i = 0; i < 11; i++)
        {
            print_char(read_buffer[i]);
        }
        print("'\n");

        // Debug: Show hex values
        print("  Expected hex: ");
        for (int i = 0; i < 11; i++)
        {
            print_hex_byte((uint8_t)test_data[i]);
            print_char(' ');
        }
        print("\n  Got hex:      ");
        for (int i = 0; i < 11; i++)
        {
            print_hex_byte((uint8_t)read_buffer[i]);
            print_char(' ');
        }
        print("\n");

        // Check integrity
        bool match = true;
        for (int i = 0; i < 11; i++)
        {
            if (read_buffer[i] != test_data[i])
            {
                match = false;
                break;
            }
        }
        print("  Integrity: ");
        print(match ? "PASS\n" : "FAIL\n");
    }

    // Test 3: Create directory
    print("\nTest 3: Create directory 'mydir'\n");
    struct EXT2DriverRequest mkdir_req;
    mkdir_req.parent_inode = 1;
    mkdir_req.name = "mydir";
    mkdir_req.name_len = 5;
    mkdir_req.buffer_size = 0; // 0 = directory
    mkdir_req.is_directory = true;

    result = write(&mkdir_req);
    print_result(result, "  write() dir");

    // Test 4: Try to create duplicate file (should fail with code 1 or -1)
    print("\nTest 4: Create duplicate 'test.txt' (should fail)\n");
    result = write(&write_req);
    print_result(result, "  write() dup");
    if (result == 1 || result == -1)
    {
        print("  Expected failure: PASS\n");
    }

    // Test 5: Create file in subdirectory (will fail if not implemented)
    print("\nTest 5: Read directory 'mydir'\n");
    struct EXT2DriverRequest readdir_req;
    char dir_buffer[512];
    readdir_req.parent_inode = 1;
    readdir_req.name = "mydir";
    readdir_req.name_len = 5;
    readdir_req.buf = dir_buffer;
    readdir_req.buffer_size = 512;

    result = read_directory(&readdir_req);
    print_result(result, "  read_directory()");

    // Test 6: Delete file
    print("\nTest 6: Delete file 'test.txt'\n");
    struct EXT2DriverRequest del_req;
    del_req.parent_inode = 1;
    del_req.name = "test.txt";
    del_req.name_len = 8;
    del_req.is_directory = false;

    result = delete(del_req);
    print_result(result, "  delete() file");

    // Test 7: Try to read deleted file (should fail with code 3 or -1 - not found)
    print("\nTest 7: Read deleted 'test.txt' (should fail)\n");
    memset(read_buffer, 0, 512); // Clear buffer to verify read fails
    result = read(read_req);
    print_result(result, "  read() deleted");
    if (result == 3 || result == -1)
    {
        print("  Expected failure: PASS\n");
    }
    else
    {
        print("  Expected failure: FAIL (got code 0x");
        print_hex_byte(result);
        print(")\n");
    }

    // Test 8: Delete directory
    print("\nTest 8: Delete directory 'mydir'\n");
    del_req.name = "mydir";
    del_req.name_len = 5;
    del_req.is_directory = true;

    result = delete(del_req);
    print_result(result, "  delete() dir");

    // Test 9: Create multiple files
    print("\nTest 9: Create multiple files\n");
    char *filenames[] = {"file1.txt", "file2.txt", "file3.txt"};
    uint8_t name_lens[] = {9, 9, 9};
    char data1[] = "Data 1";
    char data2[] = "Data 2";
    char data3[] = "Data 3";
    char *datas[] = {data1, data2, data3};
    uint32_t data_lens[] = {6, 6, 6};

    for (int i = 0; i < 3; i++)
    {
        struct EXT2DriverRequest write_multi_req;
        write_multi_req.parent_inode = 1;
        write_multi_req.name = filenames[i];
        write_multi_req.name_len = name_lens[i];
        write_multi_req.buf = datas[i];
        write_multi_req.buffer_size = data_lens[i];
        write_multi_req.is_directory = false;

        result = write(&write_multi_req);
        print("  ");
        print(filenames[i]);
        print(": ");
        if (result == 0)
        {
            print("OK\n");
        }
        else
        {
            print("FAIL (code: 0x");
            print_hex_byte(result);
            print(")\n");
        }
    }

    // Test 10: Create large file requiring single indirect blocks
    print("\nTest 10: Large file (single indirect blocks)\n");
    print("  Creating 7KB file (13 blocks)...\n");

    // Allocate buffer for large file (7KB = 7168 bytes = 14 blocks, but we'll use 13)
    static char large_buffer[6656]; // 13 * 512 = 6656 bytes
    for (int i = 0; i < 6656; i++)
    {
        large_buffer[i] = (char)('A' + (i % 26));
    }

    struct EXT2DriverRequest large_write_req;
    large_write_req.parent_inode = 1;
    large_write_req.name = "large.txt";
    large_write_req.name_len = 9;
    large_write_req.buf = large_buffer;
    large_write_req.buffer_size = 6656;
    large_write_req.is_directory = false;

    result = write(&large_write_req);
    print_result(result, "  write() large");

    if (result == 0)
    {
        // Read back and verify
        static char large_read_buffer[6656];
        memset(large_read_buffer, 0, 6656);

        struct EXT2DriverRequest large_read_req;
        large_read_req.parent_inode = 1;
        large_read_req.name = "large.txt";
        large_read_req.name_len = 9;
        large_read_req.buf = large_read_buffer;
        large_read_req.buffer_size = 6656;

        result = read(large_read_req);
        print_result(result, "  read() large");

        if (result == 0)
        {
            // Verify first 100 bytes
            bool match = true;
            for (int i = 0; i < 100; i++)
            {
                if (large_read_buffer[i] != large_buffer[i])
                {
                    match = false;
                    break;
                }
            }

            print("  First 100 bytes: ");
            print(match ? "PASS\n" : "FAIL\n");

            // Show sample of data
            print("  Sample data: ");
            for (int i = 0; i < 20; i++)
            {
                print_char(large_read_buffer[i]);
            }
            print("...\n");
        }

        // Delete the large file
        struct EXT2DriverRequest large_del_req;
        large_del_req.parent_inode = 1;
        large_del_req.name = "large.txt";
        large_del_req.name_len = 9;
        large_del_req.is_directory = false;

        result = delete(large_del_req);
        print_result(result, "  delete() large");
    }

    // Test 11: Test file spanning direct and indirect blocks
    print("\nTest 11: File with exactly 12 blocks (direct only)\n");
    static char direct_buffer[6144]; // 12 * 512 = 6144 bytes
    for (int i = 0; i < 6144; i++)
    {
        direct_buffer[i] = (char)('0' + (i % 10));
    }

    struct EXT2DriverRequest direct_write_req;
    direct_write_req.parent_inode = 1;
    direct_write_req.name = "direct.dat";
    direct_write_req.name_len = 10;
    direct_write_req.buf = direct_buffer;
    direct_write_req.buffer_size = 6144;
    direct_write_req.is_directory = false;

    result = write(&direct_write_req);
    print_result(result, "  write() direct");

    if (result == 0)
    {
        static char direct_read_buffer[6144];
        memset(direct_read_buffer, 0, 6144);

        struct EXT2DriverRequest direct_read_req;
        direct_read_req.parent_inode = 1;
        direct_read_req.name = "direct.dat";
        direct_read_req.name_len = 10;
        direct_read_req.buf = direct_read_buffer;
        direct_read_req.buffer_size = 6144;

        result = read(direct_read_req);
        print_result(result, "  read() direct");

        if (result == 0)
        {
            bool match = true;
            int first_mismatch = -1;
            for (int i = 0; i < 6144; i++)
            {
                if (direct_read_buffer[i] != direct_buffer[i])
                {
                    match = false;
                    if (first_mismatch == -1)
                    {
                        first_mismatch = i;
                    }
                    break;
                }
            }
            print("  Full integrity: ");
            print(match ? "PASS\n" : "FAIL\n");

            if (!match && first_mismatch != -1)
            {
                print("  First mismatch at byte: 0x");
                print_hex_byte((first_mismatch >> 8) & 0xFF);
                print_hex_byte(first_mismatch & 0xFF);
                print(" (expected: 0x");
                print_hex_byte((uint8_t)direct_buffer[first_mismatch]);
                print(", got: 0x");
                print_hex_byte((uint8_t)direct_read_buffer[first_mismatch]);
                print(")\n");
            }
        }

        // Delete
        struct EXT2DriverRequest direct_del_req;
        direct_del_req.parent_inode = 1;
        direct_del_req.name = "direct.dat";
        direct_del_req.name_len = 10;
        direct_del_req.is_directory = false;

        result = delete(direct_del_req);
        print_result(result, "  delete() direct");
    }

    print("\n=== Testing Complete ===\n");
    print("Check results above.\n");
}

void kernel_setup(void)
{
    load_gdt(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    activate_keyboard_interrupt();
    keyboard_state_activate();
    framebuffer_clear();
    framebuffer_set_cursor(0, 0);
    heap_init();  // Initialize kernel heap
    wallpaper_init();  // Initialize wallpaper system
    initialize_filesystem_ext2();
    gdt_install_tss();
    set_tss_register();
    set_tss_kernel_current_stack();

    paging_allocate_user_page_frame(&_paging_kernel_page_directory, (uint8_t *)0);

    struct EXT2DriverRequest request = {
        .buf = (uint8_t *)0,
        .name = "shell",
        .name_len = 5,
        .parent_inode = 1,
        .buffer_size = 0x100000,
    };
    int8_t read_result = read(request);

    if (read_result != 0)
    {
        print("Failed to load initial user program 'shell'. Halting.\n");
        print("Error code: 0x");
        print_hex_byte(read_result);
        while (true)
            ;
    }

    // Create init process and execute it
    process_create_user_process(request);
    scheduler_init();
    scheduler_switch_to_next_process();

    while (true)
        ;
}