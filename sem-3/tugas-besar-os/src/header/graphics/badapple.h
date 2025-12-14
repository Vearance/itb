/**
 * Bad Apple Animation for HuTaOS
 * Runs in kernel mode with 30 FPS on 320x200 VGA display
 * 
 * Animation can be stopped with CTRL+C
 */

#ifndef _BADAPPLE_H
#define _BADAPPLE_H

#include <stdint.h>
#include <stdbool.h>

// Frame dimensions are defined in programs/badapple.h (auto-generated)
// BADAPPLE_FRAME_COUNT, BADAPPLE_FRAME_WIDTH, BADAPPLE_FRAME_HEIGHT, BADAPPLE_BYTES_PER_FRAME

// Display dimensions
#define BADAPPLE_DISPLAY_WIDTH  320
#define BADAPPLE_DISPLAY_HEIGHT 200
#define BADAPPLE_FRAME_PIXELS (BADAPPLE_DISPLAY_WIDTH * BADAPPLE_DISPLAY_HEIGHT)
#define BADAPPLE_FRAME_COUNT 6573


// Animation settings
#define BADAPPLE_FPS            30
#define BADAPPLE_FRAME_DELAY_MS 33  // ~33ms per frame for 30 FPS

// Colors for rendering
#define BADAPPLE_COLOR_BLACK    0x00
#define BADAPPLE_COLOR_WHITE    0x0F

/**
 * Run the Bad Apple animation
 * This function runs in kernel mode and displays the animation
 * fullscreen on the VGA display (320x200).
 * 
 * The animation can be stopped by pressing CTRL+C.
 * 
 * @return 0 on success (completed or interrupted), -1 on error
 */
int badapple_run(void);

/**
 * Render a single frame of the animation
 * Scales the 64x64 1-bit frame to 320x200 display
 * 
 * @param frame_index Index of the frame to render (0 to BADAPPLE_FRAME_COUNT-1)
 */
void badapple_render_frame(uint32_t frame_index);

/**
 * Check if the user pressed CTRL+C to stop the animation
 * 
 * @return true if CTRL+C was pressed, false otherwise
 */
bool badapple_check_interrupt(void);

/**
 * Simple delay function for frame timing
 * Uses busy-wait loop calibrated for approximately the given milliseconds
 * 
 * @param ms Milliseconds to delay
 */
void badapple_delay_ms(uint32_t ms);

#endif // _BADAPPLE_H