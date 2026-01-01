#include <stdint.h>

// This is our global seed variable
static uint8_t entropy_seed = 0x64; 

uint8_t fast_rand(void) {
    uint8_t feedback;
    // XOR the current seed with the VIC-II Raster Low-Byte ($D012)
    // This adds hardware "jitter" to every single call
    entropy_seed ^= *((volatile uint8_t*)0xD012);

    feedback = ((entropy_seed >> 0) ^ (entropy_seed >> 2) ^ (entropy_seed >> 3) ^ (entropy_seed >> 5)) & 1;
    entropy_seed = (entropy_seed >> 1) | (feedback << 7);
    return entropy_seed;
}

void init_random(void) {
    // 1. Setup SID Voice 3 for Noise
    *((volatile uint8_t*)0xD412) = 0x80; // Noise Waveform
    *((volatile uint8_t*)0xD40E) = 0xFF; // Freq Low
    *((volatile uint8_t*)0xD40F) = 0xFF; // Freq High

    // 2. Mix hardware noise with a system timer for the initial seed
    // $D41B is the SID noise output, $DC04 is CIA Timer A
    entropy_seed = (*((volatile uint8_t*)0xD41B)) ^ (*((volatile uint8_t*)0xDC04));
    
    // 3. Optional: Run the generator a few times to "warm it up"
    for(uint8_t i = 0; i < 8; i++) {
        fast_rand();
    }
}

// Helper to get a number within a specific range (e.g., 0 to max-1)
uint8_t rand_range(uint8_t max) {
    return fast_rand() % max;
}