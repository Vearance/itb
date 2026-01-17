#include "header/driver/disk.h"
#include "header/cpu/portio.h"

static void ATA_busy_wait() {
    while (in(0x1F7) & ATA_STATUS_BSY);
}

static void ATA_DRQ_wait() {
    while (!(in(0x1F7) & ATA_STATUS_DRQ));
}

/**
 * Check ATA status for errors
 * @return true if error detected, false otherwise
 */
static bool ATA_check_error() {
    uint8_t status = in(0x1F7);
    return (status & ATA_STATUS_ERR) || (status & ATA_STATUS_DF);
}

void read_blocks(void *ptr, uint32_t logical_block_address, uint8_t block_count) {
    // Validate parameters
    if (ptr == NULL || block_count == 0) {
        return;
    }

    ATA_busy_wait();
    out(0x1F6, 0xE0 | ((logical_block_address >> 24) & 0xF));
    out(0x1F2, block_count);
    out(0x1F3, (uint8_t) logical_block_address);
    out(0x1F4, (uint8_t) (logical_block_address >> 8));
    out(0x1F5, (uint8_t) (logical_block_address >> 16));
    out(0x1F7, 0x20);

    uint16_t *target = (uint16_t*) ptr;
    for (uint32_t i = 0; i < block_count; i++) {
        ATA_busy_wait();
        ATA_DRQ_wait();
        
        // Check for errors before reading
        if (ATA_check_error()) {
            // On error, we can't return error code due to void signature
            // But we should not continue reading potentially corrupted data
            return;
        }
        
        for (uint32_t j = 0; j < HALF_BLOCK_SIZE; j++)
            target[j] = in16(0x1F0);
        target += HALF_BLOCK_SIZE;
    }
}

void write_blocks(const void *ptr, uint32_t logical_block_address, uint8_t block_count) {
    // Validate parameters
    if (ptr == NULL || block_count == 0) {
        return;
    }

    ATA_busy_wait();
    out(0x1F6, 0xE0 | ((logical_block_address >> 24) & 0xF));
    out(0x1F2, block_count);
    out(0x1F3, (uint8_t) logical_block_address);
    out(0x1F4, (uint8_t) (logical_block_address >> 8));
    out(0x1F5, (uint8_t) (logical_block_address >> 16));
    out(0x1F7, 0x30);

    for (uint32_t i = 0; i < block_count; i++) {
        ATA_busy_wait();
        ATA_DRQ_wait();
        
        // Check for errors before writing
        if (ATA_check_error()) {
            // On error, stop writing to prevent data corruption
            return;
        }
        
        for (uint32_t j = 0; j < HALF_BLOCK_SIZE; j++)
            out16(0x1F0, ((uint16_t*) ptr)[HALF_BLOCK_SIZE*i + j]);
    }
}
