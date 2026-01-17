#ifndef _AUDIO_H
#define _AUDIO_H

#include <stdint.h>
#include <stdbool.h>

/* -- PC Speaker / PIT (Programmable Interval Timer) Constants -- */

// PIT (8254) I/O ports
#define PIT_CHANNEL_0       0x40    // Channel 0 data port (system timer)
#define PIT_CHANNEL_1       0x41    // Channel 1 data port (DRAM refresh - obsolete)
#define PIT_CHANNEL_2       0x42    // Channel 2 data port (PC Speaker)
#define PIT_COMMAND         0x43    // Mode/Command register

// PC Speaker control port (directly controls speaker via bit 0 and 1)
#define PC_SPEAKER_PORT     0x61

// PIT base frequency in Hz (1.193182 MHz)
#define PIT_BASE_FREQUENCY  1193182

// PIT command byte for channel 2 square wave mode
// Bits: 10 11 011 0
//       ^^ ^^ ^^^ ^
//       |  |  |   +-- Binary mode (0 = binary, 1 = BCD)
//       |  |  +------ Mode 3: Square wave generator
//       |  +--------- Access mode: lobyte/hibyte (16-bit)
//       +------------ Select channel 2
#define PIT_CMD_CHANNEL2    0xB6

/* -- Musical Note Frequencies (Hz) -- */

// Octave 4 (middle octave)
#define NOTE_C4     262
#define NOTE_CS4    277     // C# / Db
#define NOTE_D4     294
#define NOTE_DS4    311     // D# / Eb
#define NOTE_E4     330
#define NOTE_F4     349
#define NOTE_FS4    370     // F# / Gb
#define NOTE_G4     392
#define NOTE_GS4    415     // G# / Ab
#define NOTE_A4     440     // Standard tuning pitch
#define NOTE_AS4    466     // A# / Bb
#define NOTE_B4     494

// Octave 5
#define NOTE_C5     523
#define NOTE_CS5    554
#define NOTE_D5     587
#define NOTE_DS5    622
#define NOTE_E5     659
#define NOTE_F5     698
#define NOTE_FS5    740
#define NOTE_G5     784
#define NOTE_GS5    831
#define NOTE_A5     880
#define NOTE_AS5    932
#define NOTE_B5     988

// Octave 3
#define NOTE_C3     131
#define NOTE_D3     147
#define NOTE_E3     165
#define NOTE_F3     175
#define NOTE_G3     196
#define NOTE_A3     220
#define NOTE_B3     247

// Rest (no sound)
#define NOTE_REST   0

/* -- Audio Driver State -- */

/**
 * AudioDriverState - Contains audio driver state information
 * @param speaker_enabled  Whether the PC speaker is currently enabled
 * @param current_freq     Current frequency being played (0 if silent)
 */
struct AudioDriverState {
    bool     speaker_enabled;
    uint32_t current_freq;
} __attribute__((packed));


/* -- Driver Interfaces -- */

/**
 * Initialize the audio driver.
 * Sets up PIT channel 2 for square wave generation.
 */
void audio_init(void);

/**
 * Play a tone at the specified frequency.
 * Uses PC Speaker via PIT channel 2.
 * 
 * @param frequency Frequency in Hz (20-20000 Hz recommended)
 *                  Use NOTE_* defines for musical notes
 */
void audio_play_tone(uint32_t frequency);

/**
 * Stop any currently playing sound.
 * Disables the PC speaker output.
 */
void audio_stop(void);

/**
 * Play a beep sound with specified frequency and duration.
 * This is a blocking function that plays and then stops.
 * 
 * @param frequency Frequency in Hz
 * @param duration  Duration in milliseconds (approximate)
 */
void audio_beep(uint32_t frequency, uint32_t duration);

/**
 * Play a simple system beep (default frequency and duration).
 * Convenience function for notification sounds.
 */
void audio_system_beep(void);

/**
 * Check if the speaker is currently playing a sound.
 * 
 * @return true if speaker is active, false otherwise
 */
bool audio_is_playing(void);

/**
 * Get the current frequency being played.
 * 
 * @return Current frequency in Hz, or 0 if not playing
 */
uint32_t audio_get_frequency(void);

/**
 * Simple delay function for audio timing.
 * Note: This is a busy-wait delay, not precise.
 * 
 * @param milliseconds Approximate delay in milliseconds
 */
void audio_delay(uint32_t milliseconds);

#endif
