// map.h -- tile/map API
#ifndef JTTP_MAP_H
#define JTTP_MAP_H

#define MAP_W 20
#define MAP_H 11

// --- Tile ID Definitions (From CharPad) ---
#define TILE_GRASS 0x00
#define TILE_PLAYER 0x01  // To be ignored in draw_world
#define TILE_HOME 0x02
#define TILE_FOREST_A 0x03
#define TILE_FOREST_B 0x04
#define TILE_DUNGEON 0x05
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

void draw_meta_tile(unsigned char tx, unsigned char ty, unsigned char tile_id);
void draw_world(void);
void init_player_position(void);
int is_walkable(unsigned char tile_id);

#endif // JTTP_MAP_H
