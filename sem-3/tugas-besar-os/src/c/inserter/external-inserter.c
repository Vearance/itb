#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "header/driver/ext2.h"
#include "header/driver/disk.h"
#include "header/stdlib/string.h"

// Local helper macro (matches ext2.c)
#define SECTORS_TO_BLOCKS(sectors) ((sectors) / (BLOCK_SIZE / 512))

// In-memory image buffers
static uint8_t *image_storage;
static uint8_t *file_buffer;

static bool find_child_inode(uint32_t parent_inode, const char *name, uint8_t name_len, uint32_t *child_inode, uint8_t *file_type) {
    struct EXT2Inode parent = get_inode(parent_inode);
    if (!(parent.i_mode & EXT2_S_IFDIR)) return false;

    uint32_t parent_blocks_count = SECTORS_TO_BLOCKS(parent.i_blocks);
    for (uint32_t blk = 0; blk < parent_blocks_count; blk++) {
        uint32_t phys = get_physical_block(&parent, blk);
        if (phys == UINT32_MAX || phys == 0) break;

        struct BlockBuffer dir_buf;
        read_blocks(&dir_buf, phys, 1);

        struct EXT2DirectoryEntry *entry = get_directory_entry(&dir_buf, 0);
        uint32_t off = 0;
        while (off < BLOCK_SIZE) {
            if (entry->rec_len == 0) break;
            if (entry->inode != 0 && entry->name_len == name_len && memcmp(get_entry_name(entry), name, name_len) == 0) {
                if (child_inode) *child_inode = entry->inode;
                if (file_type) *file_type = entry->file_type;
                return true;
            }
            off += entry->rec_len;
            if (off >= BLOCK_SIZE) break;
            entry = get_next_directory_entry(entry);
        }
    }
    return false;
}

static uint32_t ensure_path(uint32_t start_inode, const char *path) {
    const char *p = path;
    uint32_t current = start_inode;

    while (*p) {
        // Skip leading slashes
        while (*p == '/') p++;
        if (*p == '\0') break;

        // Extract component
        const char *start = p;
        while (*p && *p != '/') p++;
        uint8_t len = (uint8_t)(p - start);
        bool is_last = (*p == '\0');

        if (len == 0) continue;

        uint32_t child_inode = 0;
        uint8_t type = 0;
        bool exists = find_child_inode(current, start, len, &child_inode, &type);

        if (is_last) {
            // For the last component, return parent inode; caller will create file
            return current;
        }

        if (exists && type == EXT2_FT_DIR) {
            current = child_inode;
            continue;
        }

        if (!exists) {
            // Create missing directory
            struct EXT2DriverRequest mkdir_req = {
                .buf = NULL,
                .name = (char *)start,
                .name_len = len,
                .parent_inode = current,
                .buffer_size = 0,
                .is_directory = true
            };
            int rc = write(&mkdir_req);
            if (rc != 0) {
                printf("Error: failed to create directory %.*s (code %d)\n", len, start, rc);
                return 0;
            }
            // Re-scan to fetch newly created inode
            if (!find_child_inode(current, start, len, &child_inode, &type) || type != EXT2_FT_DIR) {
                printf("Error: directory %.*s not found after creation\n", len, start);
                return 0;
            }
            current = child_inode;
        } else {
            // Exists but not a directory
            printf("Error: %.*s exists and is not a directory\n", len, start);
            return 0;
        }
    }

    return current;
}

void read_blocks(void *ptr, uint32_t logical_block_address, uint8_t block_count) {
    for (int i = 0; i < block_count; i++) {
        memcpy(
            (uint8_t*) ptr + BLOCK_SIZE*i, 
            image_storage + BLOCK_SIZE*(logical_block_address+i), 
            BLOCK_SIZE
        );
    }
}

void write_blocks(const void *ptr, uint32_t logical_block_address, uint8_t block_count) {
    for (int i = 0; i < block_count; i++) {
        memcpy(
            image_storage + BLOCK_SIZE*(logical_block_address+i), 
            (uint8_t*) ptr + BLOCK_SIZE*i, 
            BLOCK_SIZE
        );
    }
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "inserter: ./inserter <file to insert> <parent cluster index> <storage>\n");
        exit(1);
    }

    // Read storage into memory, requiring 512 MB memory
    image_storage = malloc(512*1024*1024);
    file_buffer   = malloc(512*1024*1024);
    FILE *fptr    = fopen(argv[3], "r");
    if (fptr != NULL) {
        fread(image_storage, 512*1024*1024, 1, fptr);
        fclose(fptr);
    } else {
        // If file doesn't exist, initialize image_storage to zero
        memset(image_storage, 0, 512*1024*1024);
    }

    // Read target file, assuming file is less than 512 MiB
    FILE *fptr_target = fopen(argv[1], "r");
    size_t filesize   = 0;
    if (fptr_target == NULL)
        filesize = 0;
    else {
        fread(file_buffer, 512*1024*1024, 1, fptr_target);
        fseek(fptr_target, 0, SEEK_END);
        filesize = ftell(fptr_target);
        fclose(fptr_target);
    }

    printf("Filename : %s\n",  argv[1]);
    printf("Filesize : %ld bytes\n", filesize);

    // EXT2 operations
    initialize_filesystem_ext2();
    // Resolve/ensure directory path
    char *full_path = argv[1];
    uint8_t filename_length = (uint8_t)strlen(full_path);
    printf("Filename       : %s\n", full_path);
    printf("Filename length: %d\n", filename_length);

    uint32_t parent_inode = 0;
    sscanf(argv[2], "%u", &parent_inode);

    uint32_t target_parent = ensure_path(parent_inode, full_path);
    if (target_parent == 0) {
        printf("Path resolution/creation failed\n");
        return 1;
    }

    // Extract leaf name after last '/'
    char *leaf = strrchr(full_path, '/');
    leaf = (leaf ? leaf + 1 : full_path);
    uint8_t leaf_len = (uint8_t)strlen(leaf);

    struct EXT2DriverRequest request;
    struct EXT2DriverRequest reqread;
    uint8_t *read_buffer = malloc(4 * 1024 * 1024);

    request.buf = file_buffer;
    request.buffer_size = filesize;
    request.name = leaf;
    request.name_len = leaf_len;
    request.is_directory = false;
    request.parent_inode = target_parent;

    reqread = request;
    reqread.buf = read_buffer;
    int retcode = read(reqread);
    if (retcode == 0)
    {
        bool same = true;
        for (uint32_t i = 0; i < filesize; i++)
        {
            if (read_buffer[i] != file_buffer[i])
            {
                printf("not same\n");
                same = false;
                break;
            }
        }
        if (same)
        {
            printf("same\n");
        }
    }

    bool is_replace = false;
    retcode = write(&request);
    if (retcode == 1 && is_replace)
    {
        retcode = delete (request);
        retcode = write(&request);
    }
    if (retcode == 0)
        puts("Write success");
    else if (retcode == 1)
        puts("Error: File/folder name already exist");
    else if (retcode == 2)
        puts("Error: Invalid parent node index");
    else
        puts("Error: Unknown error");

    // Write image in memory into original, overwrite them
    fptr = fopen(argv[3], "w");
    fwrite(image_storage, 512 * 1024 * 1024, 1, fptr);
    fclose(fptr);

    return 0;
}