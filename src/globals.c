#include "globals.h"
#include "map.h"
#include "player.h"

static unsigned char game_mode = MODE_MAP;  // game always starts in MODE_MAP
static uint8_t current_dungeon = NOT_FOUND;
static uint8_t saved_ow_x = 0;
static uint8_t saved_ow_y = 0;
static uint8_t have_saved_ow = 0;

unsigned char get_game_mode(void) {
    return game_mode;
}

void switch_to_overworld(void) {
    game_mode = MODE_MAP;
    current_dungeon = NOT_FOUND;
    if (have_saved_ow) {
        player_set_pos(saved_ow_x, saved_ow_y);
        have_saved_ow = 0;
    }
}

uint8_t get_current_dungeon(void) {
    return current_dungeon;
}

void switch_to_fog_of_war(uint8_t d_idx) {
    if(d_idx == NOT_FOUND) return; // invalid dungeon index
    // Save overworld player position so we can restore it when exiting fog.
    saved_ow_x = player_get_x();
    saved_ow_y = player_get_y();
    have_saved_ow = 1;
    game_mode = MODE_FOG;
    current_dungeon = d_idx;
}