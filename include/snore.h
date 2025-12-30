// snore.h -- dragon snore sequence control
#ifndef JTTP_SNORE_H
#define JTTP_SNORE_H

// Start the snore (inhale -> exhale cycle)
void snore_start(void);

// Call every frame to advance snore timing
void snore_update(void);

#endif // JTTP_SNORE_H
