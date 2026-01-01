#include "globals.h"
#include "map.h"

static unsigned char game_mode = MODE_MAP;  // game always starts in MODE_MAP
static uint8_t current_dungeon = NOT_FOUND;

unsigned char get_game_mode(void) {
    return game_mode;
}

void switch_to_overworld(void) {
    game_mode = MODE_MAP;
    current_dungeon = NOT_FOUND;
}

uint8_t get_current_dungeon(void) {
    return current_dungeon;
}

void switch_to_fog_of_war(uint8_t d_idx) {
    if(d_idx == NOT_FOUND) return; // invalid dungeon index
    
    game_mode = MODE_FOG;
    current_dungeon = d_idx;
}