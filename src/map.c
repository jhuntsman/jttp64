#include "map.h"

#include <conio.h>
#include <globals.h>
#include <stdint.h>
#include <stdlib.h>

#include "assets.h"
#include "graphics.h"
#include "player.h"
#include "utils.h"

#define MAX_DUNGEONS 32

#pragma data(registry)
static Dungeon dungeon_registry[MAX_DUNGEONS];
static uint8_t dungeon_count = 0;
#pragma data(data)

const unsigned char type_tile_ids[] = {
    TILE_DUNGEON_UNKNOWN, TILE_DUNGEON_LVL1, TILE_DUNGEON_LVL2,
    TILE_DUNGEON_LVL3,
    TILE_DUNGEON_LVL4};  // Tile ids for for dungeon types 0-4

void draw_meta_tile(unsigned char tx, unsigned char ty, unsigned char tile_id) {
    const unsigned int screen_offset = (ty * 80) + (tx * 2);
    const unsigned int data_offset = tile_id * 4;

    const unsigned char c1 = chartileset_data[data_offset];
    const unsigned char c2 = chartileset_data[data_offset + 1];
    const unsigned char c3 = chartileset_data[data_offset + 2];
    const unsigned char c4 = chartileset_data[data_offset + 3];

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

uint8_t get_dungeon_index_at(unsigned char x, unsigned char y) {
    for (uint8_t i = 0; i < dungeon_count; i++) {
        if (dungeon_registry[i].x == x && dungeon_registry[i].y == y) {
            return i;  // Return the index (0-MAX_DUNGEONS-1)
        }
    }
    return NOT_FOUND;
}

void reveal_dungeon_visual(uint8_t d_idx) {
    uint8_t type = dungeon_registry[d_idx].type;

    if (dungeon_registry[d_idx].is_cleared ||
        !dungeon_registry[d_idx].is_identified) {
        type = 0;  // Safe/Cleared dungeons are always black
    }

    // Draw the dungeon tile
    draw_meta_tile(dungeon_registry[d_idx].x, dungeon_registry[d_idx].y,
                   type_tile_ids[type]);
}

void update_nearby_dungeons(unsigned char player_x, unsigned char player_y) {
    for (uint8_t i = 0; i < dungeon_count; i++) {
        if (!dungeon_registry[i].is_identified) {
            // Calculate absolute difference in X and Y
            // Casting to int8_t to handle negative results before abs()
            int8_t dx = abs((int8_t)dungeon_registry[i].x - (int8_t)player_x);
            int8_t dy = abs((int8_t)dungeon_registry[i].y - (int8_t)player_y);

            // If player is within 1 tile (including diagonals)
            if (dx <= 1 && dy <= 1) {
                dungeon_registry[i].is_identified = 1;
                reveal_dungeon_visual(i);
            }
        }
    }
}

void init_dungeon_registry() {
    // Distribution: 4x Type 1, 2x Type 2, 1x Type 3, 1x Type 4
    static const uint8_t weights[8] = {1, 1, 1, 1, 2, 2, 3, 4};
    
    dungeon_count = 0;
    
    // mark registry empty
    for (uint8_t i = 0; i < MAX_DUNGEONS; i++) {
        dungeon_registry[i].x = 0xFF;
        dungeon_registry[i].y = 0xFF;
        dungeon_registry[i].type = 0;
        dungeon_registry[i].is_identified = 0;
        dungeon_registry[i].is_cleared = 0;
    }

    uint8_t dungeon_index = 0;
    uint8_t range_size = (MAX_DUNGEONS - 17) + 1;
    uint8_t dungeon_max = (fast_rand() % range_size) + 17;

    while (dungeon_count < dungeon_max) {
        for (unsigned char y = 0; y < MAP_H; y++) {
            for (unsigned char x = 0; x < MAP_W; x++) {
                unsigned char tid = map_data[y * MAP_W + x];

                if (tid != TILE_DUNGEON_UNKNOWN) continue;
                if (fast_rand() % 4 != 0) continue; 

                dungeon_registry[dungeon_index].x = x;
                dungeon_registry[dungeon_index].y = y;

                // --- GUARANTEED VARIETY LOGIC ---
                if (dungeon_index < 4) {
                    // Force the first 4 placements to be one of each type
                    // (Index 0=Type 1, 1=Type 2, 2=Type 3, 3=Type 4)
                    dungeon_registry[dungeon_index].type = dungeon_index + 1;
                } else {
                    // All subsequent dungeons use the weighted table
                    dungeon_registry[dungeon_index].type = weights[fast_rand() & 0x07];
                }
                // --------------------------------

                dungeon_registry[dungeon_index].is_identified = 0;
                dungeon_registry[dungeon_index].is_cleared = 0;

                dungeon_count++;
                dungeon_index++;
                if (dungeon_count >= dungeon_max) return;
            }
        }
    }
}

void draw_world(void) {
    for (unsigned char y = 0; y < MAP_H; y++) {
        for (unsigned char x = 0; x < MAP_W; x++) {
            unsigned char tid = map_data[y * MAP_W + x];

            switch (tid) {
                case TILE_PLAYER:
                    draw_meta_tile(x, y, TILE_GRASS);
                    break;
                case TILE_DUNGEON_UNKNOWN: {
                    uint8_t d_idx = get_dungeon_index_at(x, y);
                    if (d_idx == NOT_FOUND) {
                        draw_meta_tile(x, y, TILE_MOUNTAIN);
                        break;
                    }
                    reveal_dungeon_visual(d_idx);
                } break;
                default:
                    draw_meta_tile(x, y, tid);
                    break;
            }
        }
    }
}

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

unsigned char is_walkable(unsigned char x, unsigned char y,
                          unsigned char tile_id) {
    switch (tile_id) {
        case TILE_MOUNTAIN:
            return 0;

        case TILE_FOREST_A:
        case TILE_FOREST_B:
            return player_has_axe();

        case TILE_WALL_A:
        case TILE_WALL_B:
            return 0;

        case TILE_GATE_A:
        case TILE_GATE_B:
        case TILE_GATE_C:
        case TILE_GATE_D:
            return player_has_key();

        case TILE_WATER_A:
        case TILE_WATER_B:
        case TILE_WATER_C:
        case TILE_WATER_D:
            return player_has_boat();

        case TILE_DUNGEON_UNKNOWN:
            // if dungeon exists here, walkable
            return get_dungeon_index_at(x, y) != NOT_FOUND;

        default:
            return 1;  // By default, walkable
    }
}

uint8_t can_enter_dungeon(unsigned char x, unsigned char y) {
    uint8_t d_idx = get_dungeon_index_at(x, y);
    if (d_idx == NOT_FOUND) return NOT_FOUND;

    if(dungeon_registry[d_idx].is_cleared) {
        return NOT_FOUND;  // Cannot enter uncleared dungeon
    }
    return d_idx;  // Can enter, return dungeon index
}

void clear_dungeon(uint8_t d_idx) {
    if (d_idx == NOT_FOUND) return;

    dungeon_registry[d_idx].is_cleared = 1;    
}   
