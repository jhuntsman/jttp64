#include <string.h>

#include "assets.h"
#include "graphics.h"
#include "map.h"
#include "globals.h"

void draw_meta_tile(unsigned char tx, unsigned char ty, unsigned char tile_id) {
    unsigned int screen_offset = (ty * 80) + (tx * 2);
    unsigned int data_offset = tile_id * 4;

    unsigned char c1 = chartileset_data[data_offset];
    unsigned char c2 = chartileset_data[data_offset + 1];
    unsigned char c3 = chartileset_data[data_offset + 2];
    unsigned char c4 = chartileset_data[data_offset + 3];
    unsigned char col = chartileset_attrib_L1_data[tile_id] & 0x0F;

    SCREEN_RAM[screen_offset] = c1;
    SCREEN_RAM[screen_offset + 1] = c2;
    SCREEN_RAM[screen_offset + 40] = c3;
    SCREEN_RAM[screen_offset + 41] = c4;

    COLOR_RAM[screen_offset] = col;
    COLOR_RAM[screen_offset + 1] = col;
    COLOR_RAM[screen_offset + 40] = col;
    COLOR_RAM[screen_offset + 41] = col;
}

void draw_world(void) {
    for (unsigned char y = 0; y < MAP_H; y++) {
        for (unsigned char x = 0; x < MAP_W; x++) {
            unsigned char tid = map_data[y * MAP_W + x];

            if (tid == TILE_PLAYER) tid = TILE_GRASS;

            draw_meta_tile(x, y, tid);
        }
    }
}

#include "player.h"

void init_player_position(void) {
    for (unsigned char y = 0; y < MAP_H; y++) {
        for (unsigned char x = 0; x < MAP_W; x++) {
            if (map_data[y * MAP_W + x] == TILE_HOME) {
                player_set_pos(x, y + 1);  // Start 1 tile below HOME
                return;
            }
        }
    }
}

int is_walkable(unsigned char tile_id) {
    if (tile_id == TILE_MOUNTAIN || tile_id == TILE_FOREST_A ||
        tile_id == TILE_FOREST_B || tile_id == TILE_WALL_A ||
        tile_id == TILE_WALL_B || tile_id == TILE_GATE_A ||
        tile_id == TILE_GATE_B || tile_id == TILE_GATE_C ||
        tile_id == TILE_GATE_D || tile_id == TILE_WATER_A ||
        tile_id == TILE_WATER_B || tile_id == TILE_WATER_C ||
        tile_id == TILE_WATER_D) {
        return 0;
    }
    return 1;
}
