#include "header/usermode/commands/bg.h"
#include "header/interrupt/interrupt.h"
#include "header/stdlib/string.h"
#include "header/usermode/user-shell.h"
#include "header/driver/ext2.h"
#include <stdint.h>

// External print functions from user-shell
extern void print_string(const char *str);
extern void print_string_color(const char *str, uint8_t color);
extern void flush_output_buffer(void);
extern void refresh_screen(void);

// Locate a child directory inode by name inside parent
static uint32_t find_child_dir_inode(uint32_t parent_inode, const char *name, uint8_t name_len)
{
    uint8_t dir_buf[BLOCK_SIZE * 4];
    struct EXT2DriverRequest request = {
        .buf = dir_buf,
        .parent_inode = parent_inode,
        .buffer_size = sizeof(dir_buf),
        .is_directory = true};

    int32_t retcode = 0;
    syscall(SYS_READ_DIR, (uint32_t)&request, (uint32_t)&retcode, 0);
    if (retcode != 0)
        return 0;

    uint32_t offset = 0;
    while (offset < sizeof(dir_buf))
    {
        struct EXT2DirectoryEntry *entry = (struct EXT2DirectoryEntry *)&dir_buf[offset];
        if (entry->inode == 0 || entry->rec_len == 0)
            break;

        if (entry->name_len == name_len)
        {
            const char *entry_name = get_entry_name(entry);
            if (entry->file_type == EXT2_FT_DIR && memcmp(entry_name, name, name_len) == 0)
            {
                return entry->inode;
            }
        }
        offset += entry->rec_len;
    }

    return 0;
}

// Resolve a directory path (absolute or relative to base_inode) to its inode
static uint32_t resolve_directory_inode(const char *path, uint32_t base_inode)
{
    if (!path || strlen(path) == 0)
    {
        return base_inode;
    }

    char path_copy[BG_ENV_MAX];
    uint32_t len = strlen(path);
    if (len >= BG_ENV_MAX)
    {
        return 0;
    }
    memcpy(path_copy, path, len + 1);

    uint32_t current = (path_copy[0] == '/') ? 1 : base_inode;
    char *p = path_copy;
    if (path_copy[0] == '/')
        p++;

    while (*p)
    {
        while (*p == '/')
            p++;
        if (*p == '\0')
            break;

        char *start = p;
        while (*p && *p != '/')
            p++;

        char saved = *p;
        *p = '\0';
        uint8_t name_len = (uint8_t)strlen(start);
        uint32_t child = find_child_dir_inode(current, start, name_len);
        *p = saved;

        if (child == 0)
        {
            return 0;
        }
        current = child;
        if (*p == '/')
            p++;
    }

    return current;
}

// Split path into parent inode and leaf name
static bool resolve_parent_and_leaf(const char *full_path, uint32_t cwd_inode, uint32_t *parent_out, char *leaf_out, uint8_t *leaf_len_out)
{
    if (!full_path || !parent_out || !leaf_out || !leaf_len_out)
        return false;

    size_t path_len = strlen(full_path);
    if (path_len == 0 || path_len >= BG_ENV_MAX)
        return false;

    const char *last_slash = strrchr(full_path, '/');
    if (last_slash == NULL)
    {
        uint32_t leaf_len = (uint32_t)path_len;
        memcpy(leaf_out, full_path, leaf_len);
        leaf_out[leaf_len] = '\0';
        *leaf_len_out = (uint8_t)leaf_len;
        *parent_out = cwd_inode;
        return true;
    }

    // Directory part (may be empty for root)
    size_t dir_len = (size_t)(last_slash - full_path);
    if (dir_len >= BG_ENV_MAX)
        return false;

    char dir_part[BG_ENV_MAX];
    memcpy(dir_part, full_path, dir_len);
    dir_part[dir_len] = '\0';

    const char *leaf = last_slash + 1;
    if (*leaf == '\0')
        return false;

    uint32_t base = (full_path[0] == '/') ? 1u : cwd_inode;
    uint32_t parent_inode = resolve_directory_inode(dir_part, base);
    if (parent_inode == 0)
        return false;

    uint32_t leaf_len = strlen(leaf);
    if (leaf_len >= BG_ENV_MAX)
        return false;
    memcpy(leaf_out, leaf, leaf_len);
    leaf_out[leaf_len] = '\0';
    *leaf_len_out = (uint8_t)leaf_len;
    *parent_out = parent_inode;
    return true;
}

static int8_t load_wallpaper(uint32_t parent_inode, const char *name)
{
    uint32_t result = 0;
    syscall(SYS_GFX_LOAD_WALLPAPER_FS, parent_inode, (uint32_t)name, (uint32_t)&result);
    return (int8_t)result;
}

int8_t bg_command(const char *arg)
{
    if (arg == NULL || strlen(arg) == 0)
    {
        print_string("Usage: bg <wallpaper-file>\n");
        flush_output_buffer();
        return -1;
    }

    char leaf_name[BG_ENV_MAX];
    int8_t result = -1;
    bool attempted_env = false;

    // If user supplied a path with '/', resolve it first
    if (strchr(arg, '/'))
    {
        uint8_t leaf_len = 0;
        uint32_t target_parent = current_dir_inode;
        if (!resolve_parent_and_leaf(arg, current_dir_inode, &target_parent, leaf_name, &leaf_len))
        {
            print_string_color("bg: invalid path\n", 0x0C);
            flush_output_buffer();
            return -1;
        }
        result = load_wallpaper(target_parent, leaf_name);
    }
    else
    {
        // Try BG environment search paths first (colon-separated)
        if (strlen(bg_env) > 0)
        {
            char env_copy[BG_ENV_MAX];
            uint32_t env_len = strlen(bg_env);
            if (env_len >= BG_ENV_MAX)
                env_len = BG_ENV_MAX - 1;
            memcpy(env_copy, bg_env, env_len);
            env_copy[env_len] = '\0';

            char *p = env_copy;
            while (*p)
            {
                char *start = p;
                while (*p && *p != ':')
                    p++;
                char saved = *p;
                *p = '\0';

                if (strlen(start) > 0)
                {
                    uint32_t dir_inode = resolve_directory_inode(start, 1);
                    if (dir_inode != 0)
                    {
                        result = load_wallpaper(dir_inode, arg);
                        attempted_env = true;
                        if (result == 0)
                        {
                            memcpy(leaf_name, arg, strlen(arg) + 1);
                            break;
                        }
                    }
                }

                *p = saved;
                if (*p == ':')
                    p++;
            }
        }

        // Fallback: current directory
        if (result != 0)
        {
            memcpy(leaf_name, arg, strlen(arg) + 1);
            result = load_wallpaper(current_dir_inode, leaf_name);
        }
    }

    if (result == 0)
    {
        refresh_screen();
        print_string("Wallpaper loaded from ");
        print_string(leaf_name);
        print_string("\n");
    }
    else
    {
        if (attempted_env && strlen(bg_env) > 0)
        {
            print_string_color("bg: failed to load wallpaper (checked BG paths)\n", 0x0C);
        }
        else
        {
            print_string_color("bg: failed to load wallpaper\n", 0x0C);
        }
    }

    flush_output_buffer();
    return result;
}
