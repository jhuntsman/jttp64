#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

void init_random(void);
uint8_t fast_rand(void);
uint8_t rand_range(uint8_t max);

#endif  // UTILS_H