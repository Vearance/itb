#ifndef _BG_H
#define _BG_H

#include <stdint.h>

/**
 * Change the wallpaper/background from a filesystem file (320x200 raw 8-bit)
 * @param arg File name (relative to current directory)
 * @return 0 on success, non-zero on failure
 */
int8_t bg_command(const char *arg);

#endif // _BG_H
