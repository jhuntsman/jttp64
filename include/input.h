// input.h -- joystick/input handling API
#ifndef JTTP_INPUT_H
#define JTTP_INPUT_H

#include "map.h"

// Movement repeat timing
#define MOVE_DELAY_INITIAL 15
#define MOVE_DELAY_REPEAT 6

// Border flash duration (frames)
#define BORDER_DURATION 10

#define RESET_GAME 0x0A

unsigned char handle_map_input(void);
unsigned char handle_fog_input(void);

// Collision helper declared so input.c can call the project's collision check
unsigned char is_walkable(unsigned char x, unsigned char y, unsigned char tile_id);
 

#endif // JTTP_INPUT_H
