#include <string.h>
#include <stdint.h>
#include <conio.h>
#include "assets.h"
#include "graphics.h"
#include "utils.h"
#include "player.h"

/* Use the overworld meta-tile drawer for a sanity check. */
extern void draw_meta_tile(unsigned char tx, unsigned char ty, unsigned char tile_id);

// Viewport size in meta-tiles (each meta-tile is 2x2 screen chars)
#define VIEW_W 20
#define VIEW_H 12

// tile types
#define TILE_FLOOR  0x24
#define TILE_CONNECTOR 0x25
#define TILE_REVEAL 0x26
#define TILE_SPAWN  0x27
#define TILE_WALL 0x00
#define TILE_BLACK  0x00

// Connection Bitmasks
#define CONN_N 0x01
#define CONN_S 0x02
#define CONN_E 0x04
#define CONN_W 0x08
#define FLAG_REVEALED 0x80
#define EMPTY_ROOM 0xFF

typedef struct {
    uint8_t template_id;
    uint8_t flags; // High bit = Revealed, Low bits = Connections
} RoomState;

#define GRID_SIZE 3
#define SLOT_SIZE 11
#define ROOM_SIZE 9
#define ROOM_OFFSET 1       // Padding of 1 tile around rooms
#define WORLD_SIZE (GRID_SIZE * SLOT_SIZE) // (GRID_SIZE * SLOT_SIZE)
#define TOTAL_TEMPLATES 21 // 7x3 templates in your metadata
#define GRAPH_SIZE (GRID_SIZE * GRID_SIZE)

#pragma data(data)
RoomState dungeon_graph[GRAPH_SIZE];
uint8_t cam_x, cam_y;
uint8_t player_x, player_y;

// Collision helper for fog/dungeon tiles: returns non-zero if the tile is
// crawlable. Uses dungeon tile constants defined in this file.
unsigned char is_crawlable(unsigned char x, unsigned char y, unsigned char tile_id) {
    (void)x; (void)y;
    unsigned char t = tile_id & 0x3F;
    return t == TILE_FLOOR || t == TILE_CONNECTOR || t == TILE_REVEAL || t == TILE_SPAWN;
}

void draw_dungeon_tile(unsigned char tx, unsigned char ty, unsigned char tile_id) {
    unsigned char draw_tile = tile_id;
    if (tile_id == TILE_SPAWN || tile_id == TILE_REVEAL || tile_id == TILE_CONNECTOR) {
        draw_tile = TILE_FLOOR;
    }

    // Sanity check: delegate to the existing `draw_meta_tile` used by the
    // overworld. If overworld and dungeon use the same tileset/charset this
    // should produce identical results — useful to isolate the bug.
    draw_meta_tile(tx, ty, draw_tile);
}

void update_camera(void) {
    // Player - (ViewportWidth / 2)
    int16_t tx = (int16_t)player_x - 10; 
    // Player - (ViewportHeight / 2)
    int16_t ty = (int16_t)player_y - 6; 

    // IMPORTANT: Ensure ty doesn't push the room into the "half-row" at the bottom
    while (tx < 0) tx += WORLD_SIZE;
    while (ty < 0) ty += WORLD_SIZE;
    
    cam_x = (uint8_t)(tx % WORLD_SIZE);
    cam_y = (uint8_t)(ty % WORLD_SIZE);    
}

void sync_room_connectors(uint8_t room_idx) {
    RoomState *r = &dungeon_graph[room_idx];
    uint16_t t_base = (uint16_t)r->template_id;
    
    // Calculate the 4 door positions in the 7x3 master sheet
    uint8_t tx = t_base % 7;
    uint8_t ty = t_base / 7;
    uint16_t sheet_x = tx * 9;
    uint16_t sheet_y = ty * 9;

    // We check the 9x9 template for TILE_FLOOR at the mid-edges
    // North: (4,0), South: (4,8), West: (0,4), East: (8,4)
    
    r->flags = FLAG_REVEALED; // Start fresh

    if (dungeon_room_data[(sheet_y + 0) * 63 + (sheet_x + 4)] != TILE_WALL) 
        r->flags |= CONN_N;
    if (dungeon_room_data[(sheet_y + 8) * 63 + (sheet_x + 4)] != TILE_WALL) 
        r->flags |= CONN_S;
    if (dungeon_room_data[(sheet_y + 4) * 63 + (sheet_x + 0)] != TILE_WALL) 
        r->flags |= CONN_W;
    if (dungeon_room_data[(sheet_y + 4) * 63 + (sheet_x + 8)] != TILE_WALL) 
        r->flags |= CONN_E;
}

void find_player_spawn(uint8_t room_idx) {
    uint8_t lx, ly;
    RoomState *r = &dungeon_graph[room_idx];
    uint8_t tx = r->template_id % 7;
    uint8_t ty = r->template_id / 7;

    for (ly = 0; ly < 9; ly++) {
        for (lx = 0; lx < 9; lx++) {
            uint16_t idx = ((ty * 9) + ly) * 63 + ((tx * 9) + lx);
            if (dungeon_room_data[idx] == 0x27) { // Your spawn tile ID
                // Convert local room coord to World Coord
                // (Room index to Grid coord) + Offset + Local
                player_x = (room_idx % GRID_SIZE) * SLOT_SIZE + 1 + lx;
                player_y = (room_idx / GRID_SIZE) * SLOT_SIZE + 1 + ly;
                return;
            }
        }
    }
}

void sync_player_to_view(void) {
    // We assume your player object stores its 'view-relative' position
    player_set_pos(10, 6); 
        
    update_player_sprite_pos();
}

void init_dungeon(void) {
    uint8_t i;
    
    // 1. Wipe the entire graph
    for (i = 0; i < (GRID_SIZE * GRID_SIZE); i++) {
        dungeon_graph[i].template_id = 0xFF;
        dungeon_graph[i].flags = 0;
    }

    // 2. Pick a random template ID (0 to 20)
    // Using a basic rand() or your fast_rand()
    uint8_t random_template = fast_rand() % 21; 

    // 3. Place it in the center slot (Index 4 for 3x3)
    dungeon_graph[4].template_id = random_template;
    
    // 4. Sync the room logic
    sync_room_connectors(4);
    find_player_spawn(4);    
    update_camera();     
    sync_player_to_view();
}

uint8_t get_tile_at_scaled(uint8_t tx, uint8_t ty, uint8_t gx, uint8_t gy, uint8_t lx, uint8_t ly) {
    RoomState *r = &dungeon_graph[(gy * GRID_SIZE) + gx];
    
    if (!(r->flags & FLAG_REVEALED) || r->template_id == 0xFF) return TILE_WALL;

    // Room is at local 1,1 to 9,9
    if (lx >= 1 && lx <= 9 && ly >= 1 && ly <= 9) {
        uint8_t room_lx = lx - 1; 
        uint8_t room_ly = ly - 1;

        // Calculate which room in the 7x3 grid we are using
        uint8_t template_x = r->template_id % 7; // which column (0-6)
        uint8_t template_y = r->template_id / 7; // which row (0-2)

        // Calculate coordinates in the master room-sheet (63 tiles wide)
        uint16_t master_x = (uint16_t)template_x * 9 + room_lx;
        uint16_t master_y = (uint16_t)template_y * 9 + room_ly;

        // Final 1D index = (y * total_width) + x
        // total_width is 63 (7 rooms * 9 tiles)
        uint16_t final_idx = (master_y * 63) + master_x;

        return dungeon_room_data[final_idx];
    }

    // Corridor Logic (Slot middle is 5)
    // Only check for connectors if we are on the center line (5)
    // AND inside the 1-tile buffer zones (0 or 10)

    // Vertical Hallway lane
    if (lx == 5) {
        // Is it the North buffer?
        if (ly == 0 && (r->flags & CONN_N)) return TILE_FLOOR;
        // Is it the South buffer?
        if (ly == 10 && (r->flags & CONN_S)) return TILE_FLOOR;
    }

    // Horizontal Hallway lane
    if (ly == 5) {
        // Is it the West buffer?
        if (lx == 0 && (r->flags & CONN_W)) return TILE_FLOOR;
        // Is it the East buffer?
        if (lx == 10 && (r->flags & CONN_E)) return TILE_FLOOR;
    }

    return TILE_WALL;
}

void render_dungeon(void) {
    uint8_t screen_x, screen_y;
    for (screen_y = 0; screen_y < 12; screen_y++) {
        uint8_t world_ty = (cam_y + screen_y) % 33; // Use 33 here
        uint8_t gy = world_ty / 11;
        uint8_t ly = world_ty % 11;

        for (screen_x = 0; screen_x < 20; screen_x++) {
            uint8_t world_tx = (cam_x + screen_x) % 33;
            uint8_t gx = world_tx / 11;
            uint8_t lx = world_tx % 11;

            uint8_t tile = get_tile_at_scaled(world_tx, world_ty, gx, gy, lx, ly);
            draw_dungeon_tile(screen_x, screen_y, tile);
        }
    }
}











