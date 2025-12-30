// idle.h -- idle timers and automatic events
#ifndef JTTP_IDLE_H
#define JTTP_IDLE_H

// Initialize idle subsystem
void idle_init(void);

// Reset idle timer (called on player activity)
void idle_reset(void);

// Called each frame to advance idle timers and trigger events
void idle_update(void);

#endif // JTTP_IDLE_H
