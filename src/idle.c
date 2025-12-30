#include <stdlib.h>

#include "idle.h"
#include "snore.h"

// Idle state (module-local)
static unsigned int idle_frames = 0;
static unsigned int next_breath_time = 300; // frames
static unsigned char breath_active_timer = 0;

void idle_init(void) {
    idle_frames = 0;
    next_breath_time = 900 + (rand() % 901); // 5-15s
    breath_active_timer = 0;
}

void idle_reset(void) {
    idle_frames = 0;
    next_breath_time = 900 + (rand() % 901);
}

void idle_update(void) {
    // Called once per frame from main loop
    idle_frames++;
    if (idle_frames >= next_breath_time) {
        // trigger snore sequence
        snore_start();
        idle_reset();
    }
}
