#include "header/driver/keyboard.h"
#include "header/cpu/portio.h"
#include "header/stdlib/string.h"
#include "header/interrupt/interrupt.h"
#include "header/text/framebuffer.h"

static struct KeyboardDriverState keyboard_state = {
    .keyboard_input_on = false,
    .keyboard_buffer = 0,
    .shift_pressed = false,
    .ctrl_pressed = false,
    .caps_lock_on = false,
    .read_extended_mode = false
};

const char keyboard_scancode_1_to_ascii_map[256] = {
      0, 0x1B, '1', '2', '3', '4', '5', '6',  '7', '8', '9',  '0',  '-', '=', '\b', '\t',
    'q',  'w', 'e', 'r', 't', 'y', 'u', 'i',  'o', 'p', '[',  ']', '\n',   0,  'a',  's',
    'd',  'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0, '\\',  'z', 'x',  'c',  'v',
    'b',  'n', 'm', ',', '.', '/',   0, '*',    0, ' ',   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0, '-',    0,    0,   0,  '+',    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,

      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
};

char keyboard_apply_shift_caps(char base_char) {
    bool should_uppercase = keyboard_state.shift_pressed;
    
    // Caps Lock
    if (keyboard_state.caps_lock_on && base_char >= 'a' && base_char <= 'z') {
        should_uppercase = !should_uppercase;
    }
    
    // Uppercase
    if (should_uppercase && base_char >= 'a' && base_char <= 'z') {
        return base_char - 32;
    }

    // Special characters
    if (keyboard_state.shift_pressed) {
        switch (base_char) {
            case '1': return '!';
            case '2': return '@';
            case '3': return '#';
            case '4': return '$';
            case '5': return '%';
            case '6': return '^';
            case '7': return '&';
            case '8': return '*';
            case '9': return '(';
            case '0': return ')';
            case '-': return '_';
            case '=': return '+';
            case '[': return '{';
            case ']': return '}';
            case '\\': return '|';
            case ';': return ':';
            case '\'': return '"';
            case '`': return '~';
            case ',': return '<';
            case '.': return '>';
            case '/': return '?';
            default: return base_char;
        }
    }
    
    return base_char;
}

void keyboard_state_activate(void) {
    keyboard_state.keyboard_input_on = true;
}

void keyboard_state_deactivate(void) {
    keyboard_state.keyboard_input_on = false;
}

void get_keyboard_buffer(char *buf) {
    *buf = keyboard_state.keyboard_buffer;
    keyboard_state.keyboard_buffer = 0;
}

char keyboard_read_char(void) {
    char c = 0;
    while (!c) {
        get_keyboard_buffer(&c);
    }
    return c;
}

void keyboard_read_interactive(void) {
    int row = 0, col = 0;
    // Locate cursor position after last written character
    for (int r = 0; r < FRAMEBUFFER_HEIGHT; r++) {
        for (int c = 0; c < FRAMEBUFFER_WIDTH; c++) {
            char ch = framebuffer_read(r, c);
            if (ch != 0x00 && ch != ' ') {
                row = r;
                col = c + 1;
            }
        }
    }
    // if col exceeds line width, move to next line
    if (col >= FRAMEBUFFER_WIDTH) {
        row++;
        col = 0;
    }
    
    framebuffer_set_cursor(row, col);
    
    while (true) {
        char c = keyboard_read_char();
        if (c == '\n') {
            // check if Shift + Enter for new line
            if (keyboard_state.shift_pressed) {
                // Shift + Enter - insert new line
                row++;
                col = 0;
            } else {
                // Enter -> process command (unfinished, will be continued in the next milestone)
                row++;
                col = 0;
                framebuffer_set_cursor(row, col);
                return;
            }
        } 
        else if (c == '\b') {
            // Backspace
            if (col > 0) {
                col--;
                framebuffer_write(row, col, ' ', 0xF, 0);
            } else if (row > 0) {
                // move to end of previousline
                row--;
                col = FRAMEBUFFER_WIDTH - 1;
                while (col > 0) {
                    char ch = framebuffer_read(row, col);
                    if (ch != 0x00 && ch != ' ') {
                        col++;
                        break;
                    }
                    col--;
                }
                if (col == 0) {
                    char ch = framebuffer_read(row, 0);
                    if (ch != 0x00 && ch != ' ') {
                        col = 1;
                    }
                }
            }
        }
        else if (c == ARROW_LEFT) {
            // Arrow left
            if (col > 0) {
                col--;
            } else if (row > 0) {
                // Move to end of previous line
                row--;
                // Find end of text on previous line
                col = FRAMEBUFFER_WIDTH - 1;
                while (col > 0) {
                    char ch = framebuffer_read(row, col);
                    if (ch != 0x00 && ch != ' ') {
                        col++;
                        break;
                    }
                    col--;
                }
                if (col == 0) {
                    char ch = framebuffer_read(row, 0);
                    if (ch != 0x00 && ch != ' ') {
                        col = 1;
                    }
                }
            }
        }
        else if (c == ARROW_RIGHT) {
            // arrow right
            char current_char = framebuffer_read(row, col);
            // Can move right if current position has text
            if (current_char != 0x00 && current_char != ' ') {
                col++;
                // Check if we need to wrap to next line
                if (col >= FRAMEBUFFER_WIDTH) {
                    if (row < FRAMEBUFFER_HEIGHT - 1) {
                        row++;
                        col = 0;
                    } else {
                        col = FRAMEBUFFER_WIDTH - 1;
                    }
                }
            }
            // if next position has text
            else if (col < FRAMEBUFFER_WIDTH - 1) {
                char next_char = framebuffer_read(row, col + 1);
                if (next_char != 0x00 && next_char != ' ') {
                    col++;
                }
            }
            // Or check next line if at end of current line
            else if (row < FRAMEBUFFER_HEIGHT - 1) {
                char next_line_char = framebuffer_read(row + 1, 0);
                if (next_line_char != 0x00 && next_line_char != ' ') {
                    row++;
                    col = 0;
                }
            }
        }
        /* Untuk sementara arrow up sama arrow down masih belum ada implementasinya dan rencananya
        bakalan diimplementasi untuk command history dari user */
        else if (c >= 32 && c <= 126) {
            framebuffer_write(row, col, c, 0xF, 0);
            col++;
            if (col >= FRAMEBUFFER_WIDTH) {
                row++;
                col = 0;
            }
        }
        framebuffer_set_cursor(row, col);
    }
}

void keyboard_isr(void) {
    uint8_t scancode = in(KEYBOARD_DATA_PORT);
    
    // Handle extended scancode prefix
    if (scancode == KEYBOARD_SCANCODE_EXTENDED) {
        keyboard_state.read_extended_mode = true;
    }
    // Handle key release
    else if (scancode & 0x80) {
        uint8_t released_key = scancode & 0x7F;
        if (released_key == KEYBOARD_SCANCODE_LSHIFT || released_key == KEYBOARD_SCANCODE_RSHIFT) {
            keyboard_state.shift_pressed = false;
        }
        else if (released_key == KEYBOARD_SCANCODE_LCTRL) {
            keyboard_state.ctrl_pressed = false;
        }
        keyboard_state.read_extended_mode = false;
    }
    else if (scancode == KEYBOARD_SCANCODE_LSHIFT || scancode == KEYBOARD_SCANCODE_RSHIFT) {
        keyboard_state.shift_pressed = true;
    }
    else if (scancode == KEYBOARD_SCANCODE_LCTRL) {
        keyboard_state.ctrl_pressed = true;
    }
    else if (scancode == KEYBOARD_SCANCODE_CAPS_LOCK) {
        keyboard_state.caps_lock_on = !keyboard_state.caps_lock_on;
    }
    else if (keyboard_state.keyboard_input_on) {
        char mapped_key = 0;
        if (keyboard_state.read_extended_mode) {
            switch (scancode) {
                case EXT_SCANCODE_UP:
                    mapped_key = ARROW_UP;
                    break;
                case EXT_SCANCODE_DOWN:
                    mapped_key = ARROW_DOWN;
                    break;
                case EXT_SCANCODE_LEFT:
                    mapped_key = ARROW_LEFT;
                    break;
                case EXT_SCANCODE_RIGHT:
                    mapped_key = ARROW_RIGHT;
                    break;
            }
            keyboard_state.read_extended_mode = false;
        } else {
            char base_char = keyboard_scancode_1_to_ascii_map[scancode];
            if (base_char != 0) {
                // Check for Ctrl+C (Ctrl + 'c' key, scancode 0x2E)
                if (keyboard_state.ctrl_pressed && (base_char == 'c' || base_char == 'C')) {
                    mapped_key = CTRL_C;
                } else {
                    mapped_key = keyboard_apply_shift_caps(base_char);
                }
            }
        }
        if (mapped_key != 0) {
            keyboard_state.keyboard_buffer = mapped_key;
        }
    }
    
    pic_ack(IRQ_KEYBOARD); 
}