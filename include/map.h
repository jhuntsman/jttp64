// map.h -- tile/map API
#ifndef JTTP_MAP_H
#define JTTP_MAP_H

#include <stdint.h>

#define NOT_FOUND 255

#define MAP_W 20
#define MAP_H 11

// --- Tile ID Definitions (From CharPad) ---
#define TILE_GRASS 0x00
#define TILE_PLAYER 0x01  // To be ignored in draw_world
#define TILE_HOME 0x02
#define TILE_FOREST_A 0x03
#define TILE_FOREST_B 0x04

#define TILE_DUNGEON_UNKNOWN 0x05
#define TILE_DUNGEON_LVL1 0x09
#define TILE_DUNGEON_LVL2 0x08
#define TILE_DUNGEON_LVL3 0x07
#define TILE_DUNGEON_LVL4 0x0A

#define TILE_MOUNTAIN 0x06
#define TILE_WALL_A 0x10
#define TILE_WALL_B 0x11
#define TILE_GATE_A 0x12
#define TILE_GATE_B 0x13
#define TILE_GATE_C 0x14
#define TILE_GATE_D 0x15
#define TILE_WATER_A 0x16
#define TILE_WATER_B 0x17
#define TILE_WATER_C 0x18
#define TILE_WATER_D 0x19

typedef struct {
    unsigned char x;             // 0-19
    unsigned char y;             // 0-10
    uint8_t type;          
    uint8_t is_identified; 
    uint8_t is_cleared;    
} Dungeon;

void init_dungeon_registry();
void update_nearby_dungeons(unsigned char player_x, unsigned char player_y);
uint8_t can_enter_dungeon(unsigned char x, unsigned char y);
void clear_dungeon(uint8_t d_idx);

void draw_meta_tile(unsigned char tx, unsigned char ty, unsigned char tile_id);
void draw_world(void);
void init_player_position(void);
unsigned char is_walkable(unsigned char x, unsigned char y, unsigned char tile_id);

#endif // JTTP_MAP_H
