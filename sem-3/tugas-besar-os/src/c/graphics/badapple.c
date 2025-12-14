/**
 * Bad Apple Animation for HuTaOS
 * Runs in kernel mode with 30 FPS on 320x200 VGA display
 *
 * Animation can be stopped with CTRL+C
 */

// Include frame data first (defines BADAPPLE_FRAME_COUNT, etc.)
#include "../../header/graphics/badapple.h"
#include "../../header/graphics/graphics.h"
#include "../../header/cpu/portio.h"
#include "../../header/interrupt/interrupt.h"
#include "../../header/driver/ext2.h"
#include "../../header/memory/heap.h"
#include "../../header/stdlib/string.h"

// Helper macro (matches ext2.c)
#define SECTORS_TO_BLOCKS(sectors) ((sectors) / (BLOCK_SIZE / 512))

// Keyboard port for checking CTRL+C
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_SCANCODE_LCTRL 0x1D
#define KEYBOARD_SCANCODE_C 0x2E

// Static state for interrupt detection
static volatile bool ctrl_pressed = false;
static volatile bool animation_interrupted = false;


// Find a child inode by name inside a parent directory; returns 0 if not found
static uint32_t find_child_inode(uint32_t parent_inode, const char *name, uint8_t name_len)
{
    struct EXT2Inode parent = get_inode(parent_inode);
    if (!(parent.i_mode & EXT2_S_IFDIR))
        return 0;

    uint32_t parent_blocks_count = SECTORS_TO_BLOCKS(parent.i_blocks);
    for (uint32_t blk = 0; blk < parent_blocks_count; blk++)
    {
        uint32_t phys = get_physical_block(&parent, blk);
        if (phys == UINT32_MAX || phys == 0)
            break;

        struct BlockBuffer dir_buf;
        read_blocks(&dir_buf, phys, 1);

        struct EXT2DirectoryEntry *entry = get_directory_entry(&dir_buf, 0);
        uint32_t off = 0;
        while (off < BLOCK_SIZE)
        {
            if (entry->rec_len == 0)
                break;
            if (entry->inode != 0 && entry->name_len == name_len &&
                memcmp(get_entry_name(entry), name, name_len) == 0)
            {
                return entry->inode;
            }
            off += entry->rec_len;
            if (off >= BLOCK_SIZE)
                break;
            entry = get_next_directory_entry(entry);
        }
    }
    return 0;
}


static uint8_t *badapple_frame_buf;
/**
 * Check keyboard for CTRL+C without blocking
 * Reads directly from keyboard port
 */
bool badapple_check_interrupt(void)
{
    // Check if there's data available from keyboard
    uint8_t status = in(0x64); // Keyboard status port

    if (status & 0x01)
    { // Data available
        uint8_t scancode = in(KEYBOARD_DATA_PORT);

        // Check for CTRL press/release
        if (scancode == KEYBOARD_SCANCODE_LCTRL)
        {
            ctrl_pressed = true;
        }
        else if (scancode == (KEYBOARD_SCANCODE_LCTRL | 0x80))
        {
            ctrl_pressed = false;
        }
        // Check for 'C' key while CTRL is pressed
        else if (scancode == KEYBOARD_SCANCODE_C && ctrl_pressed)
        {
            animation_interrupted = true;
            // Send ACK to PIC
            pic_ack(IRQ_KEYBOARD);
            return true;
        }

        // Send ACK to PIC for any key
        pic_ack(IRQ_KEYBOARD);
    }

    return animation_interrupted;
}

/**
 * Simple busy-wait delay
 * Calibrated for smooth animation in QEMU
 */
void badapple_delay_ms(uint32_t ms)
{
    // Tuned multiplier for ~20 FPS smooth animation
    volatile uint32_t count = ms * 80000;

    while (count > 0)
    {
        count--;
        // Check for interrupt less frequently for smoother animation
        if ((count & 0x1FFFF) == 0)
        {
            if (badapple_check_interrupt())
            {
                return;
            }
        }
    }
}

static inline uint8_t get_pixel(const uint8_t *frame_data, uint32_t width,
                                uint32_t x, uint32_t y)
{
    uint32_t pixel_idx = y * width + x;
    uint32_t byte_idx = pixel_idx / 8;
    uint32_t bit_idx = 7 - (pixel_idx % 8); // MSB is leftmost

    return (frame_data[byte_idx] >> bit_idx) & 1;
}


void badapple_render_frame(uint32_t frame_index)
{
    if (frame_index >= BADAPPLE_FRAME_COUNT)
    {
        return;
    }

    const uint8_t *frame_data = badapple_frame_buf;
    uint8_t *vga = VGA_MEMORY;

    for (uint32_t y = 0; y < BADAPPLE_DISPLAY_HEIGHT; y++)
    {
        for (uint32_t x = 0; x < BADAPPLE_DISPLAY_WIDTH; x++)
        {
            uint8_t pixel = get_pixel(frame_data, BADAPPLE_DISPLAY_WIDTH, x, y);
            uint8_t color = pixel ? BADAPPLE_COLOR_WHITE : BADAPPLE_COLOR_BLACK;

            // Write pixel to VGA memory (assuming mode 0x13)
            vga[y * BADAPPLE_DISPLAY_WIDTH + x] = color;
        }
    }
}

/**
 * Run the Bad Apple animation
 * Clears screen, plays animation at 30 FPS, returns on completion or CTRL+C
 */
int badapple_run(void)
{
    uint32_t frame_bytes = BADAPPLE_FRAME_PIXELS / 8;
    badapple_frame_buf = kzalloc(frame_bytes);
    if (badapple_frame_buf == NULL)
    {
        return -1; // Failed to allocate frame buffer
    }

    // Resolve /badapple directory and target file inside it
    uint32_t badapple_dir = find_child_inode(1, "badapple", 8);
    if (badapple_dir == 0)
    {
        kfree(badapple_frame_buf);
        badapple_frame_buf = NULL;
        return -1;
    }

    struct EXT2DriverRequest request = {
        .buf = badapple_frame_buf,
        .parent_inode = badapple_dir,
        .buffer_size = frame_bytes,
        .is_directory = false,
        .name = "badapple.bin",
        .name_len = strlen("badapple.bin")};

    // Reset interrupt state
    ctrl_pressed = false;
    animation_interrupted = false;

    // Clear screen to black
    graphics_clear(BADAPPLE_COLOR_BLACK);

    // Play each frame
    for (uint32_t frame = 0; frame < BADAPPLE_FRAME_COUNT; frame++)
    {
        // Check for interrupt before rendering
        if (badapple_check_interrupt())
        {
            break;
        }

        // Load this frame from disk into buffer
        uint32_t offset = frame * frame_bytes;
        int retcode = read_at(request, offset, frame_bytes);
        if (retcode != 0)
        {
            break;
        }

        // Render the frame using optimized method
        badapple_render_frame(frame);

        // Delay for frame timing (~30 FPS)
        badapple_delay_ms(33);

        // Check for interrupt after delay
        if (animation_interrupted)
        {
            break;
        }
    }

    // Reset state
    ctrl_pressed = false;
    animation_interrupted = false;

    kfree(badapple_frame_buf);
    badapple_frame_buf = NULL;

    return 0;
}