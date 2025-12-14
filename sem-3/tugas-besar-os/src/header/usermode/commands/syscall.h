#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <stdint.h>

#define SYS_READ                0
#define SYS_READ_DIR            1
#define SYS_WRITE               2
#define SYS_DELETE              3
#define SYS_GETCHAR             4
#define SYS_PUTCHAR             5
#define SYS_PUTS                6
#define SYS_ACT_KEYBOARD        7
#define SYS_SET_CURSOR          8
#define SYS_CLEAR_FRAMEBUFFER   9
#define SYS_SCROLL              10
#define SYS_RENAME              11
#define SYS_CREATE_PROCESS      12
#define SYS_GET_PROCESS_INFO    13
#define SYS_TERMINATE_PROCESS   14
#define SYS_TIME_GET            15
#define SYS_GFX_INIT            16
#define SYS_GFX_PUTCHAR         17
#define SYS_GFX_PUTS            18
#define SYS_GFX_CLEAR           19
#define SYS_GFX_PIXEL           20
#define SYS_GFX_SCROLL          21
#define SYS_GFX_DRAW_WALLPAPER  22
#define SYS_GFX_RESTORE_BG      23
#define SYS_KILL_PROCESS        24
#define SYS_GET_PROCESS_COUNT   25
#define SYS_GET_PROCESS_BY_INDEX 26
#define SYS_MALLOC              27
#define SYS_FREE                28
#define SYS_REALLOC             29
#define SYS_HEAP_STATS          30
#define SYS_HEAP_TEST           31
#define SYS_GFX_SET_WALLPAPER   32
#define SYS_GFX_GET_WALLPAPER_INFO 33
#define SYS_GFX_GET_WALLPAPER_NAME 34
#define SYS_GFX_LOAD_WALLPAPER_FS 35
#define SYS_BADAPPLE            36
#define SYS_BEEP                37

/**
 * @brief System call interface
 * 
 * @param eax Syscall number or first parameter
 * @param ebx Second parameter
 * @param ecx Third parameter
 * @param edx Fourth parameter
 */
void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

#endif
