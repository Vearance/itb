#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/interrupt/interrupt.h"

#define KEYBOARD_DATA_PORT          0x60
#define EXTENDED_SCANCODE_BYTE      0xE0

#define KEYBOARD_SCANCODE_LSHIFT    0x2A
#define KEYBOARD_SCANCODE_RSHIFT    0x36
#define KEYBOARD_SCANCODE_LCTRL     0x1D
#define KEYBOARD_SCANCODE_RCTRL     0x1D 
#define KEYBOARD_SCANCODE_CAPS_LOCK 0x3A
#define KEYBOARD_SCANCODE_EXTENDED  0xE0

#define EXT_SCANCODE_UP             0x48
#define EXT_SCANCODE_DOWN           0x50
#define EXT_SCANCODE_LEFT           0x4B
#define EXT_SCANCODE_RIGHT          0x4D

#define ARROW_UP                    0x10
#define ARROW_LEFT                  0x11
#define ARROW_RIGHT                 0x12
#define ARROW_DOWN                  0x13

// Control key combinations
#define CTRL_C                      0x03  // ETX (End of Text) - standard Ctrl+C

/**
 * keyboard_scancode_1_to_ascii_map[256], Convert scancode values that correspond to ASCII printables
 * How to use this array: ascii_char = k[scancode]
 * 
 * By default, QEMU using scancode set 1 (from empirical testing)
 */
extern const char keyboard_scancode_1_to_ascii_map[256];

/**
 * KeyboardDriverState - Contain all driver states
 * @param read_extended_mode Optional, can be used for signaling next read is extended scancode (ex. arrow keys)
 * @param keyboard_input_on  Indicate whether keyboard ISR is activated or not
 * @param keyboard_buffer    Storing keyboard input values in ASCII
 * @param shift_pressed      Track if shift key is currently pressed
 * @param ctrl_pressed       Track if ctrl key is currently pressed
 * @param caps_lock_on       Track if caps lock is enabled
 */
struct KeyboardDriverState {
    bool keyboard_input_on;
    char keyboard_buffer;
    bool shift_pressed;
    bool ctrl_pressed;
    bool caps_lock_on;
    bool read_extended_mode;
} __attribute((packed));

/* -- Driver Interfaces -- */

// Activate keyboard ISR / start listen keyboard & save to buffer
void keyboard_state_activate(void);

// Deactivate keyboard ISR / stop listening keyboard interrupt
void keyboard_state_deactivate(void);

// Get keyboard buffer value and flush the buffer - @param buf Pointer to char buffer
void get_keyboard_buffer(char *buf);

// Blocking function to wait and read one character from keyboard
char keyboard_read_char(void);

// Interactive keyboard input with cursor movement support (arrow keys, backspace)
void keyboard_read_interactive(void);

/* -- Keyboard Interrupt Service Routine -- */

/**
 * Handling keyboard interrupt & process scancodes into ASCII character.
 * Will start listen and process keyboard scancode if keyboard_input_on.
 */
void keyboard_isr(void);

#endif