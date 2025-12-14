#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/stdlib/string.h"
#include "header/driver/ext2.h"

// Helper macro to convert i_blocks (in 512-byte sectors) to number of BLOCK_SIZE blocks
#define SECTORS_TO_BLOCKS(sectors) ((sectors) / (BLOCK_SIZE / 512))

const uint8_t fs_signature[BLOCK_SIZE] = {
    'C', 'o', 'u', 'r', 's', 'e', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  ' ',
    'D', 'e', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'b', 'y', ' ', ' ', ' ', ' ',  ' ',
    'L', 'a', 'b', ' ', 'S', 'i', 's', 't', 'e', 'r', ' ', 'I', 'T', 'B', ' ',  ' ',
    'M', 'a', 'd', 'e', ' ', 'w', 'i', 't', 'h', ' ', '<', '3', ' ', ' ', ' ',  ' ',
    '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '2', '0', '2', '5', '\n',
    [BLOCK_SIZE-2] = 'O',
    [BLOCK_SIZE-1] = 'k',
};

static struct EXT2Superblock                g_superblock;
static struct EXT2BlockGroupDescriptorTable g_bgd_table;

// === MEMORY ===

uint32_t allocate_node(void) {
    for (uint32_t i = 0; i < GROUPS_COUNT; i++) {
        if (g_bgd_table.table[i].bg_free_inodes_count > 0) {
            struct BlockBuffer inode_bitmap;
            read_blocks(&inode_bitmap, g_bgd_table.table[i].bg_inode_bitmap, 1);
            
            uint32_t local_inode_index = find_free_bit(&inode_bitmap);
            if (local_inode_index != UINT32_MAX) {
                set_bit(&inode_bitmap, local_inode_index, true);
                write_blocks(&inode_bitmap, g_bgd_table.table[i].bg_inode_bitmap, 1);

                g_bgd_table.table[i].bg_free_inodes_count--;
                g_superblock.s_free_inodes_count--;

                return i * INODES_PER_GROUP + local_inode_index + 1;
            }
        }
    }
    return 0;  // No free inode found
}

void deallocate_node(uint32_t inode) {
    if (inode == 0) return;
    
    struct EXT2Inode node = get_inode(inode);
    
    // Deallocate all blocks used by this inode
    uint32_t blocks_count = SECTORS_TO_BLOCKS(node.i_blocks);
    deallocate_blocks(node.i_block, blocks_count);
    
    // Clear inode bitmap
    uint32_t bgd_index = inode_to_bgd(inode);
    uint32_t local_inode_index = inode_to_local(inode);
    
    struct BlockBuffer inode_bitmap;
    read_blocks(&inode_bitmap, g_bgd_table.table[bgd_index].bg_inode_bitmap, 1);
    
    set_bit(&inode_bitmap, local_inode_index, false);
    write_blocks(&inode_bitmap, g_bgd_table.table[bgd_index].bg_inode_bitmap, 1);
    
    // Update counts
    g_bgd_table.table[bgd_index].bg_free_inodes_count++;
    g_superblock.s_free_inodes_count++;
}

void sync_node(struct EXT2Inode *node, uint32_t inode_idx) {
    uint32_t bgd_index = inode_to_bgd(inode_idx);
    uint32_t local_inode_index = inode_to_local(inode_idx);
    
    uint32_t inode_table_block = g_bgd_table.table[bgd_index].bg_inode_table;
    uint32_t block_offset = local_inode_index / INODES_PER_TABLE;
    uint32_t index_in_block = local_inode_index % INODES_PER_TABLE;

    struct BlockBuffer inode_block_buffer;
    read_blocks(&inode_block_buffer, inode_table_block + block_offset, 1);
    
    struct EXT2Inode* inode_in_block = (struct EXT2Inode*)&inode_block_buffer;
    memcpy(&inode_in_block[index_in_block], node, sizeof(struct EXT2Inode));

    write_blocks(&inode_block_buffer, inode_table_block + block_offset, 1);
}

void allocate_node_blocks(void *ptr, struct EXT2Inode *node, uint32_t prefered_bgd) {
    uint32_t blocks_needed = SECTORS_TO_BLOCKS(node->i_blocks);
    uint8_t *data = (uint8_t*)ptr;
    uint32_t blocks_allocated = 0;

    // Allocate direct blocks (first 12 blocks)
    uint32_t direct_blocks = (blocks_needed < DIRECT_BLOCK_COUNT) ? blocks_needed : DIRECT_BLOCK_COUNT;
    
    for (uint32_t i = 0; i < direct_blocks; i++) {
        uint32_t block_addr = allocate_single_block(prefered_bgd);
        if (block_addr == 0) return; // Out of space
        
        node->i_block[i] = block_addr;
        
        // Write data to the specific block
        if (data != NULL) {
            struct BlockBuffer write_buffer;
            memset(&write_buffer, 0, BLOCK_SIZE);
            
            uint32_t offset = i * BLOCK_SIZE;
            uint32_t bytes_to_copy = BLOCK_SIZE;
            
            if (node->i_size < (offset + BLOCK_SIZE)) {
                if (node->i_size > offset) {
                    bytes_to_copy = node->i_size - offset;
                } else {
                    bytes_to_copy = 0;
                }
            }
            
            if (bytes_to_copy > 0) {
                memcpy(write_buffer.buf, data + offset, bytes_to_copy);
            }
            
            write_blocks(&write_buffer, block_addr, 1);
        }
        
        blocks_allocated++;
    }
    
    if (blocks_allocated >= blocks_needed) return;
    
    // Allocate single indirect blocks
    uint32_t remaining = blocks_needed - blocks_allocated;
    uint32_t single_indirect_blocks = (remaining < BLOCK_POINTERS_PER_BLOCK) ? 
                                       remaining : BLOCK_POINTERS_PER_BLOCK;
    
    if (single_indirect_blocks > 0) {
        uint32_t indirect_block = allocate_single_block(prefered_bgd);
        if (indirect_block == 0) return; // Out of space
        
        node->i_block[SINGLE_INDIRECT_INDEX] = indirect_block;
        
        uint32_t allocated = allocate_indirect_blocks(node, data, blocks_allocated,
                                                      single_indirect_blocks, 1,
                                                      indirect_block, prefered_bgd);
        blocks_allocated += allocated;
        remaining -= allocated;
    }
    
    if (blocks_allocated >= blocks_needed) return;
    
    // Allocate double indirect blocks
    uint32_t double_indirect_capacity = BLOCK_POINTERS_PER_BLOCK * BLOCK_POINTERS_PER_BLOCK;
    uint32_t double_indirect_blocks = (remaining < double_indirect_capacity) ? 
                                      remaining : double_indirect_capacity;
    
    if (double_indirect_blocks > 0) {
        uint32_t double_indirect_block = allocate_single_block(prefered_bgd);
        if (double_indirect_block == 0) return; // Out of space
        
        node->i_block[DOUBLE_INDIRECT_INDEX] = double_indirect_block;
        
        uint32_t allocated = allocate_indirect_blocks(node, data, blocks_allocated,
                                                      double_indirect_blocks, 2,
                                                      double_indirect_block, prefered_bgd);
        blocks_allocated += allocated;
        remaining -= allocated;
    }
    
    if (blocks_allocated >= blocks_needed) return;
    
    // Allocate triple indirect blocks
    if (remaining > 0) {
        uint32_t triple_indirect_block = allocate_single_block(prefered_bgd);
        if (triple_indirect_block == 0) return; // Out of space
        
        node->i_block[TRIPLE_INDIRECT_INDEX] = triple_indirect_block;
        
        allocate_indirect_blocks(node, data, blocks_allocated, remaining, 3,
                                triple_indirect_block, prefered_bgd);
    }
}

void deallocate_blocks(void *loc, uint32_t blocks) {
    uint32_t *locations = (uint32_t*)loc;
    struct BlockBuffer bitmap;
    uint32_t last_bgd = UINT32_MAX;
    bool bgd_loaded = false;
    
    deallocate_block(locations, blocks, &bitmap, 0, &last_bgd, &bgd_loaded);
    
    // Write back last bitmap if it was loaded
    if (bgd_loaded && last_bgd != UINT32_MAX) {
        write_blocks(&bitmap, g_bgd_table.table[last_bgd].bg_block_bitmap, 1);
    }
}

uint32_t allocate_single_block(uint32_t preferred_bgd) {
    uint32_t current_bgd = preferred_bgd;
    
    // Try all block groups starting from preferred
    for (uint32_t attempt = 0; attempt < GROUPS_COUNT; attempt++) {
        if (g_bgd_table.table[current_bgd].bg_free_blocks_count > 0) {
            struct BlockBuffer block_bitmap;
            read_blocks(&block_bitmap, g_bgd_table.table[current_bgd].bg_block_bitmap, 1);
            
            uint32_t local_block_index = find_free_bit(&block_bitmap);
            
            if (local_block_index != UINT32_MAX) {
                set_bit(&block_bitmap, local_block_index, true);
                write_blocks(&block_bitmap, g_bgd_table.table[current_bgd].bg_block_bitmap, 1);

                g_bgd_table.table[current_bgd].bg_free_blocks_count--;
                g_superblock.s_free_blocks_count--;
                
                uint32_t global_block_index = current_bgd * BLOCKS_PER_GROUP + local_block_index;
                return global_block_index;
            }
        }
        
        current_bgd = (current_bgd + 1) % GROUPS_COUNT;
    }
    
    return 0; // No free block found
}

uint32_t deallocate_block(uint32_t *locations, uint32_t blocks, struct BlockBuffer *bitmap, 
                          uint32_t depth, uint32_t *last_bgd, bool *bgd_loaded) {
    (void)depth; // Unused parameter
    
    // Handle direct blocks
    uint32_t direct_count = (blocks < DIRECT_BLOCK_COUNT) ? blocks : DIRECT_BLOCK_COUNT;
    
    for (uint32_t i = 0; i < direct_count; i++) {
        if (locations[i] == 0) continue;
        
        uint32_t block_idx = locations[i];
        uint32_t bgd_idx = block_idx / BLOCKS_PER_GROUP;
        uint32_t local_block_idx = block_idx % BLOCKS_PER_GROUP;
        
        // Load bitmap if needed
        if (!(*bgd_loaded) || *last_bgd != bgd_idx) {
            if ((*bgd_loaded) && *last_bgd != UINT32_MAX) {
                write_blocks(bitmap, g_bgd_table.table[*last_bgd].bg_block_bitmap, 1);
            }
            read_blocks(bitmap, g_bgd_table.table[bgd_idx].bg_block_bitmap, 1);
            *last_bgd = bgd_idx;
            *bgd_loaded = true;
        }
        
        set_bit(bitmap, local_block_idx, false);
        g_bgd_table.table[bgd_idx].bg_free_blocks_count++;
        g_superblock.s_free_blocks_count++;
    }
    
    // Handle single indirect block
    if (blocks > DIRECT_BLOCK_COUNT && locations[SINGLE_INDIRECT_INDEX] != 0) {
        deallocate_indirect_blocks(locations[SINGLE_INDIRECT_INDEX], 1, 
                                   bitmap, last_bgd, bgd_loaded);
    }
    
    // Handle double indirect block
    if (blocks > DIRECT_BLOCK_COUNT + BLOCK_POINTERS_PER_BLOCK && 
        locations[DOUBLE_INDIRECT_INDEX] != 0) {
        deallocate_indirect_blocks(locations[DOUBLE_INDIRECT_INDEX], 2,
                                   bitmap, last_bgd, bgd_loaded);
    }
    
    // Handle triple indirect block
    if (blocks > DIRECT_BLOCK_COUNT + BLOCK_POINTERS_PER_BLOCK + 
                 (BLOCK_POINTERS_PER_BLOCK * BLOCK_POINTERS_PER_BLOCK) &&
        locations[TRIPLE_INDIRECT_INDEX] != 0) {
        deallocate_indirect_blocks(locations[TRIPLE_INDIRECT_INDEX], 3,
                                   bitmap, last_bgd, bgd_loaded);
    }
    
    return *last_bgd;
}

uint32_t allocate_indirect_blocks(struct EXT2Inode *node, uint8_t *data, 
                                  uint32_t logical_start, uint32_t blocks_count,
                                  uint32_t depth, uint32_t indirect_block_addr,
                                  uint32_t preferred_bgd) {
    if (depth == 0 || blocks_count == 0) return 0;
    
    struct BlockBuffer indirect_buffer;
    memset(&indirect_buffer, 0, BLOCK_SIZE);
    uint32_t *pointers = (uint32_t*)indirect_buffer.buf;
    
    uint32_t blocks_allocated = 0;
    
    if (depth == 1) {
        // Single indirect - directly allocate data blocks
        for (uint32_t i = 0; i < blocks_count && i < BLOCK_POINTERS_PER_BLOCK; i++) {
            uint32_t block_addr = allocate_single_block(preferred_bgd);
            if (block_addr == 0) break; // Out of space
            
            pointers[i] = block_addr;
            
            // Write data to this block if provided
            if (data != NULL) {
                struct BlockBuffer write_buffer;
                memset(&write_buffer, 0, BLOCK_SIZE);
                
                uint32_t data_offset = (logical_start + i) * BLOCK_SIZE;
                uint32_t bytes_to_copy = BLOCK_SIZE;
                
                if (node->i_size < (data_offset + BLOCK_SIZE)) {
                    if (node->i_size > data_offset) {
                        bytes_to_copy = node->i_size - data_offset;
                    } else {
                        bytes_to_copy = 0;
                    }
                }
                
                if (bytes_to_copy > 0) {
                    memcpy(write_buffer.buf, data + data_offset, bytes_to_copy);
                }
                
                write_blocks(&write_buffer, block_addr, 1);
            }
            
            blocks_allocated++;
        }
    } else {
        // Double or triple indirect - recursively allocate
        uint32_t blocks_per_entry = 1;
        for (uint32_t d = 1; d < depth; d++) {
            blocks_per_entry *= BLOCK_POINTERS_PER_BLOCK;
        }
        
        uint32_t remaining = blocks_count;
        uint32_t current_logical = logical_start;
        
        for (uint32_t i = 0; i < BLOCK_POINTERS_PER_BLOCK && remaining > 0; i++) {
            uint32_t sub_indirect_block = allocate_single_block(preferred_bgd);
            if (sub_indirect_block == 0) break; // Out of space
            
            pointers[i] = sub_indirect_block;
            
            uint32_t blocks_for_this_entry = (remaining > blocks_per_entry) ? blocks_per_entry : remaining;
            uint32_t allocated = allocate_indirect_blocks(node, data, current_logical, 
                                                          blocks_for_this_entry, depth - 1,
                                                          sub_indirect_block, preferred_bgd);
            
            blocks_allocated += allocated;
            remaining -= allocated;
            current_logical += allocated;
            
            if (allocated < blocks_for_this_entry) break; // Couldn't allocate all requested
        }
    }
    
    // Write the indirect block
    write_blocks(&indirect_buffer, indirect_block_addr, 1);
    
    return blocks_allocated;
}

void deallocate_indirect_blocks(uint32_t indirect_block_addr, uint32_t depth,
                                struct BlockBuffer *bitmap, uint32_t *last_bgd,
                                bool *bgd_loaded) {
    if (indirect_block_addr == 0 || depth == 0) return;
    
    struct BlockBuffer indirect_buffer;
    read_blocks(&indirect_buffer, indirect_block_addr, 1);
    uint32_t *pointers = (uint32_t*)indirect_buffer.buf;
    
    if (depth == 1) {
        // Single indirect - deallocate data blocks
        for (uint32_t i = 0; i < BLOCK_POINTERS_PER_BLOCK; i++) {
            if (pointers[i] == 0) continue;
            
            uint32_t block_idx = pointers[i];
            uint32_t bgd_idx = block_idx / BLOCKS_PER_GROUP;
            uint32_t local_block_idx = block_idx % BLOCKS_PER_GROUP;
            
            // Load bitmap if needed
            if (!(*bgd_loaded) || *last_bgd != bgd_idx) {
                if ((*bgd_loaded) && *last_bgd != UINT32_MAX) {
                    write_blocks(bitmap, g_bgd_table.table[*last_bgd].bg_block_bitmap, 1);
                }
                read_blocks(bitmap, g_bgd_table.table[bgd_idx].bg_block_bitmap, 1);
                *last_bgd = bgd_idx;
                *bgd_loaded = true;
            }
            
            set_bit(bitmap, local_block_idx, false);
            g_bgd_table.table[bgd_idx].bg_free_blocks_count++;
            g_superblock.s_free_blocks_count++;
        }
    } else {
        // Double or triple indirect - recursively deallocate
        for (uint32_t i = 0; i < BLOCK_POINTERS_PER_BLOCK; i++) {
            if (pointers[i] != 0) {
                deallocate_indirect_blocks(pointers[i], depth - 1, bitmap, last_bgd, bgd_loaded);
            }
        }
    }
    
    // Deallocate the indirect block itself
    uint32_t bgd_idx = indirect_block_addr / BLOCKS_PER_GROUP;
    uint32_t local_block_idx = indirect_block_addr % BLOCKS_PER_GROUP;
    
    if (!(*bgd_loaded) || *last_bgd != bgd_idx) {
        if ((*bgd_loaded) && *last_bgd != UINT32_MAX) {
            write_blocks(bitmap, g_bgd_table.table[*last_bgd].bg_block_bitmap, 1);
        }
        read_blocks(bitmap, g_bgd_table.table[bgd_idx].bg_block_bitmap, 1);
        *last_bgd = bgd_idx;
        *bgd_loaded = true;
    }
    
    set_bit(bitmap, local_block_idx, false);
    g_bgd_table.table[bgd_idx].bg_free_blocks_count++;
    g_superblock.s_free_blocks_count++;
}


// === REGULAR ===

char *get_entry_name(void *entry) {
    return (char*)((struct EXT2DirectoryEntry*)entry + 1);
}

struct EXT2DirectoryEntry *get_directory_entry(void *ptr, uint32_t offset) {
    return (struct EXT2DirectoryEntry *)((uint8_t*)ptr + offset);
}

struct EXT2DirectoryEntry *get_next_directory_entry(struct EXT2DirectoryEntry *entry) {
    return (struct EXT2DirectoryEntry*)((uint8_t*)entry + entry->rec_len);
}

uint16_t get_entry_record_len(uint8_t name_len) {
    // Directory entry must be aligned on 4-byte boundaries
    // Size = sizeof(EXT2DirectoryEntry) + name_len, rounded up to multiple of 4
    uint16_t base_size = sizeof(struct EXT2DirectoryEntry) + name_len;
    return (base_size + 3) & ~3; // Round up to nearest multiple of 4
}

uint32_t get_dir_first_child_offset(void *ptr) {
    // First entry is ".", second is ".."
    // We need to skip both to get to the first actual child
    struct EXT2DirectoryEntry *current = (struct EXT2DirectoryEntry*)ptr;
    
    // Skip "."
    if (current->inode != 0 && current->name_len == 1) {
        current = get_next_directory_entry(current);
    }
    
    // Skip ".."
    if (current->inode != 0 && current->name_len == 2) {
        current = get_next_directory_entry(current);
    }
    
    return (uint32_t)((uint8_t*)current - (uint8_t*)ptr);
}


// === INITIALIZATION ===

void init_directory_table(struct EXT2Inode *node, uint32_t inode_idx, uint32_t parent_inode_idx) {
    struct BlockBuffer b;
    memset(&b, 0, BLOCK_SIZE);

    // "." entry
    struct EXT2DirectoryEntry *current_dir = (struct EXT2DirectoryEntry*) &b;
    current_dir->inode = inode_idx;
    current_dir->rec_len = 12;
    current_dir->name_len = 1;
    current_dir->file_type = EXT2_FT_DIR;
    // Name goes after the struct - get_entry_name returns pointer to location after struct
    char *name_ptr = get_entry_name(current_dir);
    memcpy(name_ptr, ".", 1);
    
    // ".." entry
    struct EXT2DirectoryEntry *parent_dir = get_next_directory_entry(current_dir);
    parent_dir->inode = parent_inode_idx;
    parent_dir->rec_len = BLOCK_SIZE - 12;
    parent_dir->name_len = 2;
    parent_dir->file_type = EXT2_FT_DIR;
    // Name goes after the struct
    name_ptr = get_entry_name(parent_dir);
    memcpy(name_ptr, "..", 2);

    allocate_node_blocks(&b, node, inode_to_bgd(inode_idx));
}

void create_ext2(void) {
/*     write_blocks(fs_signature, BOOT_SECTOR, 1);
    while (true); */
    struct BlockBuffer empty_buffer;
    memset(&empty_buffer, 0, BLOCK_SIZE);
    
    write_blocks(fs_signature, BOOT_SECTOR, 1);

    g_superblock.s_inodes_count = INODES_PER_GROUP * GROUPS_COUNT;
    g_superblock.s_blocks_count = BLOCKS_PER_GROUP * GROUPS_COUNT;
    g_superblock.s_r_blocks_count = 0;
    g_superblock.s_first_data_block = 1;
    g_superblock.s_first_ino = 1;
    g_superblock.s_blocks_per_group = BLOCKS_PER_GROUP;
    g_superblock.s_frags_per_group = BLOCKS_PER_GROUP;
    g_superblock.s_inodes_per_group = INODES_PER_GROUP;
    g_superblock.s_magic = EXT2_SUPER_MAGIC;

    // Block layout: reserve metadata inside each block group
    for (uint32_t i = 0; i < GROUPS_COUNT; i++) {
        uint32_t group_base = i * BLOCKS_PER_GROUP;
        uint32_t cursor = group_base;

        if (i == 0) {
            // Skip boot sector, superblock, and multi-block BGD table in the first group
            cursor += (2 + BGD_TABLE_BLOCKS);
        }

        g_bgd_table.table[i].bg_block_bitmap = cursor++;
        g_bgd_table.table[i].bg_inode_bitmap = cursor++;
        g_bgd_table.table[i].bg_inode_table = cursor;
        cursor += INODES_TABLE_BLOCK_COUNT;

        uint32_t reserved_blocks = 2 + INODES_TABLE_BLOCK_COUNT; // block & inode bitmap + inode table
        if (i == 0) {
            reserved_blocks += (2 + BGD_TABLE_BLOCKS); // boot, super, BGD table blocks
        }

        g_bgd_table.table[i].bg_free_blocks_count = BLOCKS_PER_GROUP - reserved_blocks;
        g_bgd_table.table[i].bg_free_inodes_count = INODES_PER_GROUP;
        g_bgd_table.table[i].bg_used_dirs_count = 0;
    }

    for (uint32_t i = 0; i < GROUPS_COUNT; i++) {
        // Initialize block bitmap
        struct BlockBuffer block_bitmap;
        memset(&block_bitmap, 0, BLOCK_SIZE);

        uint32_t group_base = i * BLOCKS_PER_GROUP;

        // Mark reserved blocks as allocated
        if (i == 0) {
            // First group: mark boot(0), superblock(1), and all BGD table blocks as allocated
            set_bit(&block_bitmap, 0, true);  // boot sector
            set_bit(&block_bitmap, 1, true);  // superblock
            for (uint32_t b = 0; b < BGD_TABLE_BLOCKS; b++) {
                set_bit(&block_bitmap, 2 + b, true);  // BGD table blocks
            }
        }

        // Mark this group's metadata blocks as allocated
        uint32_t bitmap_local = g_bgd_table.table[i].bg_block_bitmap - group_base;
        uint32_t inode_bitmap_local = g_bgd_table.table[i].bg_inode_bitmap - group_base;
        uint32_t inode_table_local = g_bgd_table.table[i].bg_inode_table - group_base;

        set_bit(&block_bitmap, bitmap_local, true);
        set_bit(&block_bitmap, inode_bitmap_local, true);
        for (uint32_t j = 0; j < INODES_TABLE_BLOCK_COUNT; j++) {
            set_bit(&block_bitmap, inode_table_local + j, true);
        }

        write_blocks(&block_bitmap, g_bgd_table.table[i].bg_block_bitmap, 1);
        write_blocks(&empty_buffer, g_bgd_table.table[i].bg_inode_bitmap, 1);
        for (uint32_t j = 0; j < INODES_TABLE_BLOCK_COUNT; j++) {
            write_blocks(&empty_buffer, g_bgd_table.table[i].bg_inode_table + j, 1);
        }
    }

    uint32_t root_inode_idx = allocate_node();
    struct EXT2Inode root_inode;
    memset(&root_inode, 0, sizeof(struct EXT2Inode));
    
    root_inode.i_mode = EXT2_S_IFDIR;
    root_inode.i_size = BLOCK_SIZE;
    root_inode.i_blocks = BLOCK_SIZE / 512;
    
    init_directory_table(&root_inode, root_inode_idx, root_inode_idx);
    sync_node(&root_inode, root_inode_idx);

    g_bgd_table.table[0].bg_used_dirs_count++;

    g_superblock.s_free_inodes_count = (INODES_PER_GROUP * GROUPS_COUNT) - 1;
    
    // Calculate total free blocks across all groups after allocating metadata and root
    g_superblock.s_free_blocks_count = 0;
    for (uint32_t i = 0; i < GROUPS_COUNT; i++) {
        g_superblock.s_free_blocks_count += g_bgd_table.table[i].bg_free_blocks_count;
    }
    g_superblock.s_free_blocks_count -= 1; // Subtract root directory block
    
    write_blocks(&g_superblock, 1, 1);
    write_blocks(&g_bgd_table, BGD_TABLE_START_BLOCK, (uint8_t)BGD_TABLE_BLOCKS);
}

void initialize_filesystem_ext2(void) {    
    if (is_empty_storage()) {
        create_ext2();
    } else {
        read_blocks(&g_superblock, 1, 1);
        read_blocks(&g_bgd_table, BGD_TABLE_START_BLOCK, (uint8_t)BGD_TABLE_BLOCKS);
    }
}


// === MAIN ===

uint32_t inode_to_bgd(uint32_t inode) {
    if (inode == 0) return 0;
    return (inode - 1) / INODES_PER_GROUP;
}

uint32_t inode_to_local(uint32_t inode) {
    if (inode == 0) return 0;
    return (inode - 1) % INODES_PER_GROUP;
}

bool is_empty_storage(void) {
    struct BlockBuffer boot_sector_buffer;
    read_blocks(&boot_sector_buffer, BOOT_SECTOR, 1);
    return memcmp(&boot_sector_buffer, fs_signature, BLOCK_SIZE) != 0;
}

bool is_directory_empty(uint32_t inode) {
    struct EXT2Inode node = get_inode(inode);
    if (!(node.i_mode & EXT2_S_IFDIR)) {
        return false;
    }
    uint32_t dir_blocks = SECTORS_TO_BLOCKS(node.i_blocks);
    if (dir_blocks == 0) dir_blocks = 1;

    for (uint32_t blk = 0; blk < dir_blocks; blk++) {
        uint32_t phys = get_physical_block(&node, blk);
        if (phys == UINT32_MAX || phys == 0) break;

        struct BlockBuffer dir_buf;
        read_blocks(&dir_buf, phys, 1);

        uint32_t offset = 0;
        while (offset < BLOCK_SIZE) {
            struct EXT2DirectoryEntry *entry = (struct EXT2DirectoryEntry *)((uint8_t *)&dir_buf + offset);
            if (entry->rec_len == 0) break;
            if (entry->inode != 0) {
                char *name = get_entry_name(entry);
                if (!(entry->name_len == 1 && name[0] == '.') &&
                    !(entry->name_len == 2 && name[0] == '.' && name[1] == '.')) {
                    return false;
                }
            }
            offset += entry->rec_len;
        }
    }
    return true;
}


// === HELPERS ===

uint32_t find_free_bit(struct BlockBuffer *bitmap) {
    for (uint32_t i = 0; i < BLOCK_SIZE; i++) {
        if (bitmap->buf[i] != 0xFF) {
            for (uint8_t j = 0; j < 8; j++) {
                if (!((bitmap->buf[i] >> j) & 1)) {
                    return i * 8 + j;
                }
            }
        }
    }
    return UINT32_MAX;  // No free bit found
}

void set_bit(struct BlockBuffer *bitmap, uint32_t bit_index, bool value) {
    uint32_t byte_index = bit_index / 8;
    uint8_t bit_offset = bit_index % 8;
    if (value) {
        bitmap->buf[byte_index] |= (1 << bit_offset);
    } else {
        bitmap->buf[byte_index] &= ~(1 << bit_offset);
    }
}

struct EXT2Inode get_inode(uint32_t inode) {
    struct EXT2Inode empty_inode;
    memset(&empty_inode, 0, sizeof(struct EXT2Inode));
    
    // Validate inode range (1 to total inodes)
    if (inode == 0 || inode > g_superblock.s_inodes_count) {
        return empty_inode; // Return empty inode on invalid range
    }
    
    uint32_t bgd_index = inode_to_bgd(inode);
    uint32_t local_inode_index = inode_to_local(inode);
    
    // Validate bgd_index
    if (bgd_index >= GROUPS_COUNT) {
        return empty_inode;
    }
    
    // Check if inode is allocated in bitmap
    struct BlockBuffer inode_bitmap;
    read_blocks(&inode_bitmap, g_bgd_table.table[bgd_index].bg_inode_bitmap, 1);
    
    uint32_t byte_index = local_inode_index / 8;
    uint8_t bit_offset = local_inode_index % 8;
    bool is_allocated = (inode_bitmap.buf[byte_index] >> bit_offset) & 1;
    
    if (!is_allocated) {
        return empty_inode; // Inode not allocated
    }
    
    uint32_t inode_table_block = g_bgd_table.table[bgd_index].bg_inode_table;
    
    // Validate inode_table_block is within disk bounds
    if (inode_table_block == 0 || inode_table_block >= g_superblock.s_blocks_count) {
        return empty_inode;
    }
    
    uint32_t block_offset = local_inode_index / INODES_PER_TABLE;
    uint32_t index_in_block = local_inode_index % INODES_PER_TABLE;

    struct BlockBuffer inode_block_buffer;
    read_blocks(&inode_block_buffer, inode_table_block + block_offset, 1);
    
    struct EXT2Inode* inode_in_block = (struct EXT2Inode*)&inode_block_buffer;
    return inode_in_block[index_in_block];
}

int get_block_level(uint32_t blocks_needed) {
    if (blocks_needed < DIRECT_BLOCK_COUNT) {
        return 0; // direct blocks
    } else if (blocks_needed < DIRECT_BLOCK_COUNT + BLOCK_POINTERS_PER_BLOCK) {
        return 1; // single indirect
    } else if (blocks_needed < DIRECT_BLOCK_COUNT + BLOCK_POINTERS_PER_BLOCK + 
                                (BLOCK_POINTERS_PER_BLOCK * BLOCK_POINTERS_PER_BLOCK)) {
        return 2; // double indirect
    } else {
        return 3; // triple indirect
    }
}

uint32_t get_physical_block(struct EXT2Inode *node, uint32_t logical_block_index) {
    // Determine which level this block belongs to based on its index
    if (logical_block_index < DIRECT_BLOCK_COUNT) {
        // Direct block
        return node->i_block[logical_block_index];
    } else if (logical_block_index < DIRECT_BLOCK_COUNT + BLOCK_POINTERS_PER_BLOCK) {
        // Single indirect
        if (node->i_block[SINGLE_INDIRECT_INDEX] == 0) return UINT32_MAX;

        struct BlockBuffer indirect_block;
        uint32_t indirect_index = logical_block_index - DIRECT_BLOCK_COUNT;
        read_blocks(&indirect_block, node->i_block[SINGLE_INDIRECT_INDEX], 1);

        uint32_t *block_pointers = (uint32_t*)indirect_block.buf;
        return block_pointers[indirect_index];
        
    } else if (logical_block_index < DIRECT_BLOCK_COUNT + BLOCK_POINTERS_PER_BLOCK + 
                                      (BLOCK_POINTERS_PER_BLOCK * BLOCK_POINTERS_PER_BLOCK)) {
        // Double indirect
        if (node->i_block[DOUBLE_INDIRECT_INDEX] == 0) return UINT32_MAX;

        struct BlockBuffer first_indirect_block;
        uint32_t offset = logical_block_index - DIRECT_BLOCK_COUNT - BLOCK_POINTERS_PER_BLOCK;
        uint32_t first_index = offset / BLOCK_POINTERS_PER_BLOCK;
        uint32_t second_index = offset % BLOCK_POINTERS_PER_BLOCK;

        read_blocks(&first_indirect_block, node->i_block[DOUBLE_INDIRECT_INDEX], 1);
        uint32_t *first_level_pointers = (uint32_t*)first_indirect_block.buf;

        if (first_level_pointers[first_index] == 0) return UINT32_MAX;

        struct BlockBuffer second_indirect_block;
        read_blocks(&second_indirect_block, first_level_pointers[first_index], 1);
        uint32_t *second_level_pointers = (uint32_t*)second_indirect_block.buf;

        return second_level_pointers[second_index];
    } else {
        // Triple indirect
        if (node->i_block[TRIPLE_INDIRECT_INDEX] == 0) return UINT32_MAX;

        struct BlockBuffer first_indirect_block;
        read_blocks(&first_indirect_block, node->i_block[TRIPLE_INDIRECT_INDEX], 1);
        uint32_t *first_level_pointers = (uint32_t*)first_indirect_block.buf;

        uint32_t offset = logical_block_index - DIRECT_BLOCK_COUNT - BLOCK_POINTERS_PER_BLOCK - 
                         (BLOCK_POINTERS_PER_BLOCK * BLOCK_POINTERS_PER_BLOCK);
        uint32_t blocks_per_double = BLOCK_POINTERS_PER_BLOCK * BLOCK_POINTERS_PER_BLOCK;
        uint32_t first_index = offset / blocks_per_double;
        uint32_t remainder = offset % blocks_per_double;
        uint32_t second_index = remainder / BLOCK_POINTERS_PER_BLOCK;
        uint32_t third_index = remainder % BLOCK_POINTERS_PER_BLOCK;

        if (first_level_pointers[first_index] == 0) return UINT32_MAX;
        struct BlockBuffer second_indirect_block;
        read_blocks(&second_indirect_block, first_level_pointers[first_index], 1);
        uint32_t *second_level_pointers = (uint32_t*)second_indirect_block.buf;

        if (second_level_pointers[second_index] == 0) return UINT32_MAX;
        struct BlockBuffer third_indirect_block;
        read_blocks(&third_indirect_block, second_level_pointers[second_index], 1);
        uint32_t *third_level_pointers = (uint32_t*)third_indirect_block.buf;

        return third_level_pointers[third_index];
    }
}

// === CRUD FUNCTIONS ===

int8_t read(struct EXT2DriverRequest request) {
    struct EXT2Inode parent = get_inode(request.parent_inode);

    if (!(parent.i_mode & EXT2_S_IFDIR)) return 4; // parent is not a directory

    bool found = false;
    struct EXT2DirectoryEntry matched_entry = {0};

    uint32_t parent_blocks_count = SECTORS_TO_BLOCKS(parent.i_blocks);
    for (uint32_t blk = 0; blk < parent_blocks_count && !found; blk++) {
        uint32_t phys = get_physical_block(&parent, blk);
        if (phys == UINT32_MAX || phys == 0) break;

        struct BlockBuffer dir_buf;
        read_blocks(&dir_buf, phys, 1);

        struct EXT2DirectoryEntry *entry = get_directory_entry(&dir_buf, 0);
        uint32_t off = 0;
        while (off < BLOCK_SIZE) {
            if (entry->rec_len == 0) break;
            if (entry->inode != 0 && entry->file_type == EXT2_FT_REG_FILE &&
                entry->name_len == request.name_len &&
                memcmp(get_entry_name(entry), request.name, request.name_len) == 0) {
                matched_entry = *entry;
                found = true;
                break;
            }
            off += entry->rec_len;
            if (off >= BLOCK_SIZE) break;
            entry = get_next_directory_entry(entry);
        }
    }

    if (!found) return 3; // not found

    struct EXT2Inode file_inode = get_inode(matched_entry.inode);

    if (!(file_inode.i_mode & EXT2_S_IFREG)) return 1; // not a file

    if (request.buffer_size < file_inode.i_size) return 2; // not enough buffer

    uint32_t blocks_to_read = SECTORS_TO_BLOCKS(file_inode.i_blocks);
    
    // Read file data block by block
    for (uint32_t i = 0; i < blocks_to_read; i++) {
        uint32_t physical_block = get_physical_block(&file_inode, i);
        if (physical_block == UINT32_MAX) break; // No more blocks
        
        struct BlockBuffer data_block;
        read_blocks(&data_block, physical_block, 1);
        
        uint32_t bytes_to_copy = BLOCK_SIZE;
        uint32_t file_offset = i * BLOCK_SIZE;
        if (file_inode.i_size < file_offset + BLOCK_SIZE) {
            if (file_inode.i_size > file_offset) {
                bytes_to_copy = file_inode.i_size - file_offset;
            } else {
                bytes_to_copy = 0;
            }
        }
        
        if (bytes_to_copy > 0) {
            memcpy((uint8_t*)request.buf + file_offset, data_block.buf, bytes_to_copy);
        }
    }
    
    // Add null terminator at the end of file content for safe string handling
    if (file_inode.i_size < request.buffer_size) {
        ((uint8_t*)request.buf)[file_inode.i_size] = '\0';
    }
    
    return 0; // success
}

int8_t read_directory(struct EXT2DriverRequest *prequest) {
    struct EXT2Inode parent = get_inode(prequest->parent_inode);

    if (!(parent.i_mode & EXT2_S_IFDIR)) return 3; // parent is not a directory

    // If name_len is 0, return the first block (legacy behavior)
    if (prequest->name_len == 0) {
        if (prequest->buffer_size < BLOCK_SIZE) return 4; // not enough buffer
        read_blocks(prequest->buf, parent.i_block[0], 1);
        return 0; // success
    }

    bool found = false;
    struct EXT2DirectoryEntry found_entry_local = {0};

    uint32_t parent_blocks_count = SECTORS_TO_BLOCKS(parent.i_blocks);
    for (uint32_t blk = 0; blk < parent_blocks_count && !found; blk++) {
        uint32_t phys = get_physical_block(&parent, blk);
        if (phys == UINT32_MAX || phys == 0) break;

        struct BlockBuffer dir_buf;
        read_blocks(&dir_buf, phys, 1);

        struct EXT2DirectoryEntry *entry = get_directory_entry(&dir_buf, 0);
        uint32_t read_offset = 0;
        while (read_offset < BLOCK_SIZE) {
            if (entry->rec_len == 0) break;
            
            if (entry->inode != 0 &&
                entry->name_len == prequest->name_len &&
                memcmp(get_entry_name(entry), prequest->name, prequest->name_len) == 0) {
                found = true;
                found_entry_local = *entry;
                break;
            }
            
            read_offset += entry->rec_len;
            if (read_offset >= BLOCK_SIZE) break;
            entry = get_next_directory_entry(entry);
        }
    }

    if (!found) return 2; // not found

    struct EXT2Inode dir_inode = get_inode(found_entry_local.inode);

    if (!(dir_inode.i_mode & EXT2_S_IFDIR)) return 1; // not a folder

    if (prequest->buffer_size < BLOCK_SIZE) return 4; // not enough buffer

    // Write found entry to buffer at offset 0
    struct EXT2DirectoryEntry *buf_entry = (struct EXT2DirectoryEntry *)prequest->buf;
    buf_entry->inode = found_entry_local.inode;
    buf_entry->rec_len = found_entry_local.rec_len;
    buf_entry->name_len = found_entry_local.name_len;
    buf_entry->file_type = found_entry_local.file_type;
    
    // Read the directory contents into the buffer (overwrites entry)
    read_blocks(prequest->buf, dir_inode.i_block[0], 1);

    return 0; // success
}

int8_t write(struct EXT2DriverRequest *request) {
    // Validate parent inode
    struct EXT2Inode parent = get_inode(request->parent_inode);
    
    // Check if parent inode is valid (i_mode should not be 0) and is a directory
    if (parent.i_mode == 0 || !(parent.i_mode & EXT2_S_IFDIR)) return 2; // parent is not a directory or invalid
    
    uint32_t parent_blocks_count = SECTORS_TO_BLOCKS(parent.i_blocks);
    if (parent_blocks_count == 0) parent_blocks_count = 1; // a directory must have at least one block

    uint16_t needed_len = get_entry_record_len(request->name_len);

    // First pass: check for duplicate names across all directory blocks
    for (uint32_t blk = 0; blk < parent_blocks_count; blk++) {
        uint32_t phys = get_physical_block(&parent, blk);
        if (phys == UINT32_MAX || phys == 0) break;

        struct BlockBuffer dir_buf;
        read_blocks(&dir_buf, phys, 1);

        struct EXT2DirectoryEntry *entry = get_directory_entry(&dir_buf, 0);
        uint32_t off = 0;
        while (off < BLOCK_SIZE) {
            if (entry->rec_len == 0) break;
            if (entry->inode != 0 &&
                entry->name_len == request->name_len &&
                memcmp(get_entry_name(entry), request->name, request->name_len) == 0) {
                return 1; // already exists
            }
            off += entry->rec_len;
            if (off >= BLOCK_SIZE) break;
            entry = get_next_directory_entry(entry);
        }
    }

    // Second pass: find slot or split space across blocks
    bool found_slot = false;
    uint32_t slot_phys = 0;
    struct BlockBuffer slot_buf;
    struct EXT2DirectoryEntry *slot_entry = NULL;

    for (uint32_t blk = 0; blk < parent_blocks_count && !found_slot; blk++) {
        uint32_t phys = get_physical_block(&parent, blk);
        if (phys == UINT32_MAX || phys == 0) break;

        struct BlockBuffer dir_buf;
        read_blocks(&dir_buf, phys, 1);

        struct EXT2DirectoryEntry *entry = get_directory_entry(&dir_buf, 0);
        uint32_t off = 0;
        while (off < BLOCK_SIZE) {
            if (entry->rec_len == 0) break;

            if (entry->inode == 0 && entry->rec_len >= needed_len) {
                // Reuse empty slot
                slot_buf = dir_buf;
                slot_entry = (struct EXT2DirectoryEntry *)((uint8_t *)&slot_buf + ((uint8_t *)entry - (uint8_t *)&dir_buf));
                slot_phys = phys;
                found_slot = true;
                break;
            }

            uint16_t actual_len = get_entry_record_len(entry->name_len);
            if (entry->rec_len >= actual_len + needed_len) {
                // Split current entry
                uint8_t *base = (uint8_t *)&dir_buf;
                uint32_t prev_offset = (uint8_t *)entry - base;
                uint32_t new_offset = prev_offset + actual_len;
                if (new_offset + needed_len <= BLOCK_SIZE) {
                    struct EXT2DirectoryEntry *new_entry = (struct EXT2DirectoryEntry *)(base + new_offset);
                    new_entry->rec_len = entry->rec_len - actual_len;
                    entry->rec_len = actual_len;
                    slot_buf = dir_buf;
                    slot_entry = (struct EXT2DirectoryEntry *)((uint8_t *)&slot_buf + (new_offset));
                    slot_phys = phys;
                    found_slot = true;
                    break;
                }
            }

            off += entry->rec_len;
            if (off >= BLOCK_SIZE) break;
            entry = get_next_directory_entry(entry);
        }
    }

    // If still no slot, allocate new directory block and append
    if (!found_slot) {
        uint32_t new_block = allocate_single_block(inode_to_bgd(request->parent_inode));
        if (new_block == 0) return 15; // no space

        uint32_t blk_index = parent_blocks_count;

        if (parent_blocks_count < DIRECT_BLOCK_COUNT) {
            parent.i_block[blk_index] = new_block;
        } else if (parent_blocks_count < DIRECT_BLOCK_COUNT + BLOCK_POINTERS_PER_BLOCK) {
            uint32_t indirect_index = parent_blocks_count - DIRECT_BLOCK_COUNT;
            if (parent.i_block[SINGLE_INDIRECT_INDEX] == 0) {
                uint32_t indirect_block = allocate_single_block(inode_to_bgd(request->parent_inode));
                if (indirect_block == 0) return 15;
                parent.i_block[SINGLE_INDIRECT_INDEX] = indirect_block;
            }
            struct BlockBuffer indirect_buffer;
            read_blocks(&indirect_buffer, parent.i_block[SINGLE_INDIRECT_INDEX], 1);
            uint32_t *pointers = (uint32_t *)indirect_buffer.buf;
            pointers[indirect_index] = new_block;
            write_blocks(&indirect_buffer, parent.i_block[SINGLE_INDIRECT_INDEX], 1);
        } else if (parent_blocks_count < DIRECT_BLOCK_COUNT + BLOCK_POINTERS_PER_BLOCK + (BLOCK_POINTERS_PER_BLOCK * BLOCK_POINTERS_PER_BLOCK)) {
            uint32_t offset = parent_blocks_count - DIRECT_BLOCK_COUNT - BLOCK_POINTERS_PER_BLOCK;
            uint32_t first_index = offset / BLOCK_POINTERS_PER_BLOCK;
            uint32_t second_index = offset % BLOCK_POINTERS_PER_BLOCK;

            if (parent.i_block[DOUBLE_INDIRECT_INDEX] == 0) {
                uint32_t double_indirect_block = allocate_single_block(inode_to_bgd(request->parent_inode));
                if (double_indirect_block == 0) return 15;
                parent.i_block[DOUBLE_INDIRECT_INDEX] = double_indirect_block;
            }

            struct BlockBuffer first_indirect_buffer;
            read_blocks(&first_indirect_buffer, parent.i_block[DOUBLE_INDIRECT_INDEX], 1);
            uint32_t *first_level_pointers = (uint32_t *)first_indirect_buffer.buf;

            if (first_level_pointers[first_index] == 0) {
                uint32_t second_indirect_block = allocate_single_block(inode_to_bgd(request->parent_inode));
                if (second_indirect_block == 0) return 15;
                first_level_pointers[first_index] = second_indirect_block;
                write_blocks(&first_indirect_buffer, parent.i_block[DOUBLE_INDIRECT_INDEX], 1);
            }

            struct BlockBuffer second_indirect_buffer;
            read_blocks(&second_indirect_buffer, first_level_pointers[first_index], 1);
            uint32_t *second_level_pointers = (uint32_t *)second_indirect_buffer.buf;
            second_level_pointers[second_index] = new_block;
            write_blocks(&second_indirect_buffer, first_level_pointers[first_index], 1);
        } else {
            // Triple indirect support
            uint32_t offset = parent_blocks_count - DIRECT_BLOCK_COUNT - BLOCK_POINTERS_PER_BLOCK - (BLOCK_POINTERS_PER_BLOCK * BLOCK_POINTERS_PER_BLOCK);
            uint32_t blocks_per_double = BLOCK_POINTERS_PER_BLOCK * BLOCK_POINTERS_PER_BLOCK;
            uint32_t first_index = offset / blocks_per_double;
            uint32_t remainder = offset % blocks_per_double;
            uint32_t second_index = remainder / BLOCK_POINTERS_PER_BLOCK;
            uint32_t third_index = remainder % BLOCK_POINTERS_PER_BLOCK;

            // Allocate triple indirect root if needed
            if (parent.i_block[TRIPLE_INDIRECT_INDEX] == 0) {
                uint32_t triple_indirect_block = allocate_single_block(inode_to_bgd(request->parent_inode));
                if (triple_indirect_block == 0) return 15;
                parent.i_block[TRIPLE_INDIRECT_INDEX] = triple_indirect_block;
                struct BlockBuffer zero_buf; memset(&zero_buf, 0, BLOCK_SIZE);
                write_blocks(&zero_buf, triple_indirect_block, 1);
            }

            // First level
            struct BlockBuffer first_indirect_buffer;
            read_blocks(&first_indirect_buffer, parent.i_block[TRIPLE_INDIRECT_INDEX], 1);
            uint32_t *first_level_pointers = (uint32_t *)first_indirect_buffer.buf;

            if (first_level_pointers[first_index] == 0) {
                uint32_t second_level_block = allocate_single_block(inode_to_bgd(request->parent_inode));
                if (second_level_block == 0) return 15;
                first_level_pointers[first_index] = second_level_block;
                struct BlockBuffer zero_buf; memset(&zero_buf, 0, BLOCK_SIZE);
                write_blocks(&zero_buf, second_level_block, 1);
                write_blocks(&first_indirect_buffer, parent.i_block[TRIPLE_INDIRECT_INDEX], 1);
            }

            // Second level
            struct BlockBuffer second_indirect_buffer;
            read_blocks(&second_indirect_buffer, first_level_pointers[first_index], 1);
            uint32_t *second_level_pointers = (uint32_t *)second_indirect_buffer.buf;

            if (second_level_pointers[second_index] == 0) {
                uint32_t third_level_block = allocate_single_block(inode_to_bgd(request->parent_inode));
                if (third_level_block == 0) return 15;
                second_level_pointers[second_index] = third_level_block;
                struct BlockBuffer zero_buf; memset(&zero_buf, 0, BLOCK_SIZE);
                write_blocks(&zero_buf, third_level_block, 1);
                write_blocks(&second_indirect_buffer, first_level_pointers[first_index], 1);
            }

            // Third level (data pointers)
            struct BlockBuffer third_indirect_buffer;
            read_blocks(&third_indirect_buffer, second_level_pointers[second_index], 1);
            uint32_t *third_level_pointers = (uint32_t *)third_indirect_buffer.buf;

            third_level_pointers[third_index] = new_block;
            write_blocks(&third_indirect_buffer, second_level_pointers[second_index], 1);
        }

        parent.i_size += BLOCK_SIZE;
        parent.i_blocks += BLOCK_SIZE / 512;
        sync_node(&parent, request->parent_inode);

        struct BlockBuffer new_dir_block;
        memset(&new_dir_block, 0, BLOCK_SIZE);
        struct EXT2DirectoryEntry *first_entry = (struct EXT2DirectoryEntry *)&new_dir_block;
        first_entry->inode = 0;
        first_entry->rec_len = BLOCK_SIZE;
        first_entry->name_len = 0;
        first_entry->file_type = 0;
        write_blocks(&new_dir_block, new_block, 1);

        slot_buf = new_dir_block;
        slot_entry = (struct EXT2DirectoryEntry *)&slot_buf;
        slot_phys = new_block;
        found_slot = true;
    }

    if (!found_slot || slot_entry == NULL || slot_phys == 0) return 15; // no space in directory
    
    // Allocate new inode
    uint32_t new_inode_idx = allocate_node();
    if (new_inode_idx == 0) return 16; // no free inode
    
    struct EXT2Inode new_inode;
    memset(&new_inode, 0, sizeof(struct EXT2Inode));
    
    if (request->is_directory) {
        // Creating a directory
        new_inode.i_mode = EXT2_S_IFDIR;
        new_inode.i_size = BLOCK_SIZE;
        new_inode.i_blocks = BLOCK_SIZE / 512;
        
        init_directory_table(&new_inode, new_inode_idx, request->parent_inode);
        sync_node(&new_inode, new_inode_idx);
        
        // Update directory count
        uint32_t bgd_idx = inode_to_bgd(new_inode_idx);
        g_bgd_table.table[bgd_idx].bg_used_dirs_count++;
        
        slot_entry->file_type = EXT2_FT_DIR;
    } else {
        // Creating a file
        new_inode.i_mode = EXT2_S_IFREG;
        new_inode.i_size = request->buffer_size;
        uint32_t blocks_needed = (request->buffer_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
        new_inode.i_blocks = blocks_needed * (BLOCK_SIZE / 512);
        
        if (request->buffer_size > 0) {
            struct BlockBuffer file_buffer;
            memset(&file_buffer, 0, BLOCK_SIZE);
            if (request->buffer_size <= BLOCK_SIZE) {
                memcpy(&file_buffer, request->buf, request->buffer_size);
                allocate_node_blocks((uint8_t *)&file_buffer, &new_inode, inode_to_bgd(new_inode_idx));
            } else {
                allocate_node_blocks(request->buf, &new_inode, inode_to_bgd(new_inode_idx));
            }
        }
        sync_node(&new_inode, new_inode_idx);
        
        slot_entry->file_type = EXT2_FT_REG_FILE;
    }
    
    // Fill directory entry in the correct block buffer
    slot_entry->inode = new_inode_idx;
    slot_entry->name_len = request->name_len;

    char *entry_name_ptr = get_entry_name(slot_entry);
    uint32_t entry_offset = (uint8_t *)slot_entry - (uint8_t *)&slot_buf;
    uint32_t name_offset = entry_offset + sizeof(struct EXT2DirectoryEntry);
    if (name_offset + request->name_len <= BLOCK_SIZE) {
        memcpy(entry_name_ptr, request->name, request->name_len);
    } else {
        return 17; // Would overflow buffer
    }
    
    // Persist directory block that actually contains the new entry
    write_blocks(&slot_buf, slot_phys, 1);
    
    // Update superblock and BGD table
    write_blocks(&g_superblock, 1, 1);
    write_blocks(&g_bgd_table, BGD_TABLE_START_BLOCK, (uint8_t)BGD_TABLE_BLOCKS);
    
    return 0; // success
}

int8_t delete(struct EXT2DriverRequest request) {
    // Validate parent inode
    struct EXT2Inode parent = get_inode(request.parent_inode);
    
    // Check if parent inode is valid (i_mode should not be 0) and is a directory
    if (parent.i_mode == 0 || !(parent.i_mode & EXT2_S_IFDIR)) return 3; // parent is not a directory or invalid
    
    // Check if parent has valid data block
    if (parent.i_block[0] == 0) return 3; // parent directory has no content
    
    // Calculate which block the entry is in and read all blocks to find the entry
    uint32_t parent_blocks_count = SECTORS_TO_BLOCKS(parent.i_blocks);
    uint32_t found_block_idx = UINT32_MAX;
    uint32_t del_offset = 0;
    struct EXT2DirectoryEntry *prev_entry = NULL;
    struct EXT2DirectoryEntry *entry = NULL;
    bool found = false;
    
    // Search through all blocks in the directory
    for (uint32_t block_idx = 0; block_idx < parent_blocks_count; block_idx++) {
        // Get the physical block number
        uint32_t physical_block = get_physical_block(&parent, block_idx);
        if (physical_block == UINT32_MAX) break; // No more blocks
        
        // Read this block
        struct BlockBuffer dir_buffer;
        read_blocks(&dir_buffer, physical_block, 1);
        
        // Search within this block
        entry = get_directory_entry(&dir_buffer, 0);
        del_offset = 0;
        prev_entry = NULL;
        
        while (del_offset < BLOCK_SIZE) {
            // Safety check
            if (entry->rec_len == 0) break;
            
            if (entry->inode != 0 &&
                entry->name_len == request.name_len &&
                memcmp(get_entry_name(entry), request.name, request.name_len) == 0) {
                found = true;
                found_block_idx = block_idx;
                break;
            }
            
            prev_entry = entry;
            del_offset += entry->rec_len;
            if (del_offset >= BLOCK_SIZE) break;
            entry = get_next_directory_entry(entry);
        }
        
        if (found) break;
    }
    
    // Need to re-read the block where the entry was found
    struct BlockBuffer dir_buffer;
    if (!found) return 1; // not found
    
    uint32_t physical_block = get_physical_block(&parent, found_block_idx);
    read_blocks(&dir_buffer, physical_block, 1);
    
    // Re-find the entry in the block buffer
    entry = get_directory_entry(&dir_buffer, 0);
    del_offset = 0;
    prev_entry = NULL;
    
    while (del_offset < BLOCK_SIZE) {
        if (entry->rec_len == 0) break;
        
        if (entry->inode != 0 &&
            entry->name_len == request.name_len &&
            memcmp(get_entry_name(entry), request.name, request.name_len) == 0) {
            break;
        }
        
        prev_entry = entry;
        del_offset += entry->rec_len;
        if (del_offset >= BLOCK_SIZE) break;
        entry = get_next_directory_entry(entry);
    }
    
    // Store the inode number before we modify the entry
    uint32_t target_inode = entry->inode;
    
    // Check if it's a directory and if it's empty
    if (request.is_directory || entry->file_type == EXT2_FT_DIR) {
        if (!is_directory_empty(target_inode)) {
            return 2; // folder is not empty
        }
        
        // Decrease directory count
        uint32_t bgd_idx = inode_to_bgd(target_inode);
        g_bgd_table.table[bgd_idx].bg_used_dirs_count--;
    }
    
    // Deallocate inode and its blocks
    deallocate_node(target_inode);
    
    // Merge with previous entry to reclaim space
    if (prev_entry != NULL && del_offset > 0) {
        // Add deleted entry's rec_len to previous entry
        prev_entry->rec_len += entry->rec_len;
    } else {
        // If this is the first entry, just mark as unused
        entry->inode = 0;
    }
    
    // Write back directory table to the correct block
    write_blocks(&dir_buffer, physical_block, 1);
    
    // Update superblock and BGD table
    write_blocks(&g_superblock, 1, 1);
    write_blocks(&g_bgd_table, BGD_TABLE_START_BLOCK, (uint8_t)BGD_TABLE_BLOCKS);
    
    return 0; // success
}

/**
 * @brief Rename a file or directory entry
 * 
 * @param request EXT2DriverRequest with:
 *        - name: old name
 *        - name_len: length of old name
 *        - parent_inode: parent directory inode
 *        - buf: pointer to new name string
 * @return 0 on success, error code otherwise
 */
int8_t rename_entry(struct EXT2DriverRequest *request) {
    // Validate parent inode
    struct EXT2Inode parent = get_inode(request->parent_inode);
    
    if (parent.i_mode == 0 || !(parent.i_mode & EXT2_S_IFDIR)) {
        return 2; // parent is not a directory or invalid
    }

    char *new_name = (char *)request->buf;
    uint32_t new_name_len = strlen(new_name);
    if (new_name_len > 255) {
        return 3; // new name too long
    }

    uint16_t needed_len = get_entry_record_len((uint8_t)new_name_len);

    uint32_t parent_blocks_count = SECTORS_TO_BLOCKS(parent.i_blocks);
    if (parent_blocks_count == 0) parent_blocks_count = 1;

    // Check if new name already exists across all blocks
    for (uint32_t blk = 0; blk < parent_blocks_count; blk++) {
        uint32_t phys = get_physical_block(&parent, blk);
        if (phys == UINT32_MAX || phys == 0) break;

        struct BlockBuffer dir_buf;
        read_blocks(&dir_buf, phys, 1);

        struct EXT2DirectoryEntry *entry = get_directory_entry(&dir_buf, 0);
        uint32_t offset = 0;
        while (offset < BLOCK_SIZE) {
            if (entry->rec_len == 0) break;
            if (entry->inode != 0 &&
                entry->name_len == new_name_len &&
                memcmp(get_entry_name(entry), new_name, new_name_len) == 0) {
                return 4; // new name already exists
            }
            offset += entry->rec_len;
            if (offset >= BLOCK_SIZE) break;
            entry = get_next_directory_entry(entry);
        }
    }

    // Find target entry across blocks
    struct BlockBuffer target_buf;
    struct EXT2DirectoryEntry *target_entry = NULL;
    uint32_t target_phys = 0;

    for (uint32_t blk = 0; blk < parent_blocks_count; blk++) {
        uint32_t phys = get_physical_block(&parent, blk);
        if (phys == UINT32_MAX || phys == 0) break;

        struct BlockBuffer dir_buf;
        read_blocks(&dir_buf, phys, 1);

        struct EXT2DirectoryEntry *entry = get_directory_entry(&dir_buf, 0);
        uint32_t offset = 0;
        while (offset < BLOCK_SIZE) {
            if (entry->rec_len == 0) break;
            if (entry->inode != 0 &&
                entry->name_len == request->name_len &&
                memcmp(get_entry_name(entry), request->name, request->name_len) == 0) {
                if (needed_len > entry->rec_len) {
                    return 3; // new name does not fit in current slot
                }
                target_buf = dir_buf;
                target_entry = (struct EXT2DirectoryEntry *)((uint8_t *)&target_buf + ((uint8_t *)entry - (uint8_t *)&dir_buf));
                target_phys = phys;
                break;
            }
            offset += entry->rec_len;
            if (offset >= BLOCK_SIZE) break;
            entry = get_next_directory_entry(entry);
        }
        if (target_entry != NULL) break;
    }

    if (target_entry == NULL) return 1; // not found

    // Update name safely
    target_entry->name_len = (uint8_t)new_name_len;
    char *entry_name = get_entry_name(target_entry);
    uint32_t entry_offset = (uint8_t *)target_entry - (uint8_t *)&target_buf;
    uint32_t name_offset = entry_offset + sizeof(struct EXT2DirectoryEntry);
    if (name_offset + new_name_len > BLOCK_SIZE) return 3; // would overflow
    memcpy(entry_name, new_name, new_name_len);

    write_blocks(&target_buf, target_phys, 1);
    return 0; // success
}

int8_t read_at(struct EXT2DriverRequest request, uint32_t offset, uint32_t length) {
    // Reuse directory lookup logic
    struct EXT2Inode parent = get_inode(request.parent_inode);
    if (!(parent.i_mode & EXT2_S_IFDIR)) return 4; // parent is not a directory

    bool found = false;
    struct EXT2DirectoryEntry matched_entry = {0};

    uint32_t parent_blocks_count = SECTORS_TO_BLOCKS(parent.i_blocks);
    for (uint32_t blk = 0; blk < parent_blocks_count && !found; blk++) {
        uint32_t phys = get_physical_block(&parent, blk);
        if (phys == UINT32_MAX || phys == 0) break;

        struct BlockBuffer dir_buf;
        read_blocks(&dir_buf, phys, 1);

        struct EXT2DirectoryEntry *entry = get_directory_entry(&dir_buf, 0);
        uint32_t off = 0;
        while (off < BLOCK_SIZE) {
            if (entry->rec_len == 0) break;
            if (entry->inode != 0 && entry->file_type == EXT2_FT_REG_FILE &&
                entry->name_len == request.name_len &&
                memcmp(get_entry_name(entry), request.name, request.name_len) == 0) {
                matched_entry = *entry;
                found = true;
                break;
            }
            off += entry->rec_len;
            if (off >= BLOCK_SIZE) break;
            entry = get_next_directory_entry(entry);
        }
    }

    if (!found) return 3; // not found

    struct EXT2Inode file_inode = get_inode(matched_entry.inode);
    if (!(file_inode.i_mode & EXT2_S_IFREG)) return 1; // not a file

    if (offset >= file_inode.i_size) return 0; // nothing to read

    if (offset + length > file_inode.i_size) {
        length = file_inode.i_size - offset;
    }

    uint32_t block_size = BLOCK_SIZE;
    uint32_t first_block = offset / block_size;
    uint32_t last_block = (offset + length - 1) / block_size;
    uint32_t bytes_copied = 0;

    for (uint32_t blk = first_block; blk <= last_block; blk++) {
        uint32_t phys = get_physical_block(&file_inode, blk);
        if (phys == UINT32_MAX) break;

        struct BlockBuffer data_block;
        read_blocks(&data_block, phys, 1);

        uint32_t block_start_offset = blk * block_size;
        uint32_t start_in_block = (offset > block_start_offset) ? (offset - block_start_offset) : 0;
        uint32_t bytes_in_block = block_size - start_in_block;
        if (bytes_in_block > (length - bytes_copied)) {
            bytes_in_block = length - bytes_copied;
        }

        memcpy((uint8_t *)request.buf + bytes_copied, data_block.buf + start_in_block, bytes_in_block);
        bytes_copied += bytes_in_block;
        if (bytes_copied >= length) break;
    }

    // Null-terminate if caller buffer has space and reading text
    if (bytes_copied < request.buffer_size) {
        ((uint8_t *)request.buf)[bytes_copied] = '\0';
    }

    return 0;
}