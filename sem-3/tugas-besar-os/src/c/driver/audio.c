#include "header/driver/audio.h"
#include "header/cpu/portio.h"

/* -- Driver State -- */
static struct AudioDriverState audio_state = {
    .speaker_enabled = false,
    .current_freq = 0
};

/* -- Private Helper Functions -- */

/**
 * Set the PIT channel 2 frequency divider.
 * The actual frequency produced = PIT_BASE_FREQUENCY / divisor
 * 
 * @param divisor 16-bit frequency divider value
 */
static void pit_set_channel2_divisor(uint16_t divisor) {
    // Send command byte to PIT (channel 2, lobyte/hibyte, square wave mode)
    out(PIT_COMMAND, PIT_CMD_CHANNEL2);
    
    // Send divisor (low byte first, then high byte)
    out(PIT_CHANNEL_2, (uint8_t)(divisor & 0xFF));         // Low byte
    out(PIT_CHANNEL_2, (uint8_t)((divisor >> 8) & 0xFF));  // High byte
}

/**
 * Enable the PC speaker output.
 * Sets bits 0 and 1 of port 0x61:
 *   Bit 0: Gate signal for PIT channel 2
 *   Bit 1: Speaker data enable
 */
static void speaker_enable(void) {
    uint8_t port_value = in(PC_SPEAKER_PORT);
    
    // Only enable if not already enabled (bits 0 and 1)
    if ((port_value & 0x03) != 0x03) {
        out(PC_SPEAKER_PORT, port_value | 0x03);
    }
    
    audio_state.speaker_enabled = true;
}

/**
 * Disable the PC speaker output.
 * Clears bits 0 and 1 of port 0x61.
 */
static void speaker_disable(void) {
    uint8_t port_value = in(PC_SPEAKER_PORT);
    
    // Clear bits 0 and 1 to disable speaker
    out(PC_SPEAKER_PORT, port_value & 0xFC);
    
    audio_state.speaker_enabled = false;
    audio_state.current_freq = 0;
}

/* -- Public Driver Interface Implementation -- */

void audio_init(void) {
    // Ensure speaker is off initially
    speaker_disable();
    
    audio_state.speaker_enabled = false;
    audio_state.current_freq = 0;
}

void audio_play_tone(uint32_t frequency) {
    // Don't play if frequency is 0 (rest) or out of audible range
    if (frequency == 0 || frequency < 20 || frequency > 20000) {
        audio_stop();
        return;
    }
    
    // Calculate the divisor for the desired frequency
    // divisor = PIT_BASE_FREQUENCY / frequency
    uint32_t divisor = PIT_BASE_FREQUENCY / frequency;
    
    // Clamp divisor to 16-bit range
    if (divisor > 0xFFFF) {
        divisor = 0xFFFF;
    }
    if (divisor < 1) {
        divisor = 1;
    }
    
    // Set the PIT channel 2 frequency
    pit_set_channel2_divisor((uint16_t)divisor);
    
    // Enable the speaker
    speaker_enable();
    
    // Update state
    audio_state.current_freq = frequency;
}

void audio_stop(void) {
    speaker_disable();
}

void audio_delay(uint32_t milliseconds) {
    // Use port 0x80 for timing - each I/O operation takes ~1 microsecond
    // So we need ~1000 I/O ops per millisecond
    // Adding extra iterations for QEMU overhead
    for (uint32_t ms = 0; ms < milliseconds; ms++) {
        for (volatile uint32_t i = 0; i < 1000; i++) {
            in(0x80);  // Each port read takes ~1 microsecond
        }
    }
}

void audio_beep(uint32_t frequency, uint32_t duration) {
    // Start playing the tone
    audio_play_tone(frequency);
    
    // Wait for the specified duration
    audio_delay(duration);
    
    // Stop the tone
    audio_stop();
}

void audio_system_beep(void) {
    // Default system beep: 1000 Hz for 100ms
    audio_beep(1000, 100);
}

bool audio_is_playing(void) {
    return audio_state.speaker_enabled;
}

uint32_t audio_get_frequency(void) {
    return audio_state.current_freq;
}
