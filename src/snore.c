#include <c64/vic.h>
#include <c64/sid.h>

#include "snore.h"
#include "sound.h"

static unsigned int snore_phase_timer = 0;
static unsigned char snore_state = 0;  // 0=Inhale,1=Exhale,2=Done

void snore_start(void) {
    snore_state = 0;
    snore_phase_timer = 120; // Inhale for 2 seconds
    play_dragon_snore(0);
}

void snore_update(void) {
    if (snore_phase_timer > 0) {
        snore_phase_timer--;

        if (snore_state == 2) {
            // Screen shake: Vibrate the 3 vertical scroll bits of $D011
            vic.ctrl1 = (vic.ctrl1 & 0xF8) | (snore_phase_timer & 0x03);
        }

        if (snore_phase_timer == 0) {
            if (snore_state == 0) {          // End Inhale
                sid.voices[0].ctrl &= 0xFE;  // Release Gate Voice 1
                sid.voices[1].ctrl &= 0xFE;  // Release Gate Voice 2
                snore_state = 1;
                snore_phase_timer = 20;
            } else if (snore_state == 1) {  // Start Exhale
                play_dragon_snore(1);
                snore_state = 2;
                snore_phase_timer = 80;
            } else if (snore_state == 2) {  // End Exhale
                sid.voices[0].ctrl &= 0xFE;
                sid.voices[1].ctrl &= 0xFE;
                vic.ctrl1 = (vic.ctrl1 & 0xF8) | 0x03;  // Reset Screen
            }
        }
    }
}
