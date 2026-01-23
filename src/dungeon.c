#include <string.h>
#include <stdint.h>
#include <conio.h>
#include <c64/vic.h>
#include "assets.h"
#include "graphics.h"
#include "utils.h"
#include "player.h"
#include "dungeon.h"

/* Use the overworld meta-tile drawer for a sanity check. */
extern void draw_meta_tile(unsigned char tx, unsigned char ty, unsigned char tile_id);

// Viewport size in meta-tiles (each meta-tile is 2x2 screen chars)
#define VIEW_W 20
#define VIEW_H 12

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
#define ROOM_TEMPLATES_WIDTH 7
#define ROOM_TEMPLATES_HEIGHT 3
#define TOTAL_TEMPLATES (ROOM_TEMPLATES_WIDTH * ROOM_TEMPLATES_HEIGHT) // 7x3 templates in your metadata
#define GRAPH_SIZE (GRID_SIZE * GRID_SIZE)

#pragma data(data)
RoomState dungeon_graph[GRAPH_SIZE];
uint8_t cam_x, cam_y;
uint8_t player_x, player_y;

// Hardware smooth scrolling state (pixel offsets 0-15 for full meta-tile)
uint8_t scroll_x_pixels = 0;  // 0-15 pixels (16 = 1 meta-tile, wraps via VIC hardware)
uint8_t scroll_y_pixels = 0;  // 0-15 pixels (16 = 1 meta-tile, wraps via VIC hardware)
uint8_t needs_screen_redraw = 0;

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
    // Camera updates are now handled directly by viewport scrolling functions.
    // This function is kept for compatibility but does nothing.
}

void apply_hardware_scroll(void) {
    // Update VIC-II hardware scroll registers for smooth scrolling
    // VIC ctrl1 bits 0-2 = Y scroll (0-7 pixels)
    // VIC ctrl2 bits 0-2 = X scroll (0-7 pixels)
    // We use modulo 8 since VIC only handles 0-7 pixel ranges
    
    // Update X scroll (bits 0-2 of $D016)
    vic.ctrl2 = (vic.ctrl2 & 0xF8) | (scroll_x_pixels & 0x07);
    
    // Update Y scroll (bits 0-2 of $D011)
    vic.ctrl1 = (vic.ctrl1 & 0xF8) | (scroll_y_pixels & 0x07);
}

void redraw_column(uint8_t screen_col) {
    // Redraw a single vertical column (screen_x position)
    uint8_t screen_y;
    
    for (screen_y = 0; screen_y < 12; screen_y++) {
        uint8_t world_tx = (uint8_t)((cam_x + screen_col + 33) % 33);
        uint8_t world_ty = (uint8_t)((cam_y + screen_y + 33) % 33);
        
        uint8_t gx = world_tx / 11;
        uint8_t gy = world_ty / 11;
        uint8_t lx = world_tx % 11;
        uint8_t ly = world_ty % 11;
        
        uint8_t tile = get_tile_at_scaled(world_tx, world_ty, gx, gy, lx, ly);
        draw_dungeon_tile(screen_col, screen_y, tile);
    }
}

void redraw_row(uint8_t screen_row) {
    // Redraw a single horizontal row (screen_y position)
    uint8_t screen_x;
    
    for (screen_x = 0; screen_x < 20; screen_x++) {
        uint8_t world_tx = (uint8_t)((cam_x + screen_x + 33) % 33);
        uint8_t world_ty = (uint8_t)((cam_y + screen_row + 33) % 33);
        
        uint8_t gx = world_tx / 11;
        uint8_t gy = world_ty / 11;
        uint8_t lx = world_tx % 11;
        uint8_t ly = world_ty % 11;
        
        uint8_t tile = get_tile_at_scaled(world_tx, world_ty, gx, gy, lx, ly);
        draw_dungeon_tile(screen_x, screen_row, tile);
    }
}

void scroll_viewport_left(void) {
    // Scroll left by 16 pixels (1 meta-tile = 2 world tiles) in a single keypress
    scroll_x_pixels += 16;
    
    if (scroll_x_pixels >= 16) {
        // Move camera and reset scroll state
        scroll_x_pixels = 0;
        cam_x = (cam_x - 2 + 33) % 33;  // Move 2 tiles left
        needs_screen_redraw = 1;
    }
    
    apply_hardware_scroll();
}

void scroll_viewport_right(void) {
    // Scroll right by 16 pixels (1 meta-tile = 2 world tiles) in a single keypress
    scroll_x_pixels += 16;
    
    if (scroll_x_pixels >= 16) {
        scroll_x_pixels = 0;
        cam_x = (cam_x + 2) % 33;  // Move 2 tiles right
        needs_screen_redraw = 1;
    }
    
    // Apply inverted scroll for right direction
    uint8_t inverted = (16 - scroll_x_pixels) & 0x07;
    vic.ctrl2 = (vic.ctrl2 & 0xF8) | inverted;
}

void scroll_viewport_up(void) {
    // Scroll up by 16 pixels (1 meta-tile = 2 world tiles) in a single keypress
    scroll_y_pixels += 16;
    
    if (scroll_y_pixels >= 16) {
        scroll_y_pixels = 0;
        cam_y = (cam_y - 2 + 33) % 33;  // Move 2 tiles up
        needs_screen_redraw = 1;
    }
    
    apply_hardware_scroll();
}

void scroll_viewport_down(void) {
    // Scroll down by 16 pixels (1 meta-tile = 2 world tiles) in a single keypress
    scroll_y_pixels += 16;
    
    if (scroll_y_pixels >= 16) {
        scroll_y_pixels = 0;
        cam_y = (cam_y + 2) % 33;  // Move 2 tiles down
        needs_screen_redraw = 1;
    }
    
    // Apply inverted scroll for down direction
    uint8_t inverted = (16 - scroll_y_pixels) & 0x07;
    vic.ctrl1 = (vic.ctrl1 & 0xF8) | inverted;
}

RoomState get_room_state_from_template(uint8_t template_id) {
    RoomState r;
    r.template_id = template_id;
    r.flags = FLAG_REVEALED; // Default to visible for this check

    // Calculate sheet positions
    uint8_t sheet_col = template_id % ROOM_TEMPLATES_WIDTH;
    uint8_t sheet_row = template_id / ROOM_TEMPLATES_WIDTH;
    uint16_t base_x = (uint16_t)sheet_col * ROOM_SIZE;
    uint16_t base_y = (uint16_t)sheet_row * ROOM_SIZE;

    // Check North (Midpoint 4, 0)
    if (dungeon_room_data[(base_y + 0) * 63 + (base_x + 4)] != TILE_WALL)
        r.flags |= CONN_N;

    // Check South (Midpoint 4, 8)
    if (dungeon_room_data[(base_y + 8) * 63 + (base_x + 4)] != TILE_WALL)
        r.flags |= CONN_S;

    // Check West (Midpoint 0, 4)
    if (dungeon_room_data[(base_y + 4) * 63 + (base_x + 0)] != TILE_WALL)
        r.flags |= CONN_W;

    // Check East (Midpoint 8, 4)
    if (dungeon_room_data[(base_y + 4) * 63 + (base_x + 8)] != TILE_WALL)
        r.flags |= CONN_E;

    return r;
}

void sync_room_connectors(uint8_t room_idx) {
    RoomState *r = &dungeon_graph[room_idx];
    uint16_t t_base = (uint16_t)r->template_id;
    
    // Calculate the 4 door positions in the 7x3 master sheet
    uint8_t tx = t_base % ROOM_TEMPLATES_WIDTH;
    uint8_t ty = t_base / ROOM_TEMPLATES_WIDTH;
    uint16_t sheet_x = tx * ROOM_SIZE;
    uint16_t sheet_y = ty * ROOM_SIZE;

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
    // Player position is now fixed in the center of the viewport.
    // Viewport scrolling is controlled by camera position (cam_x, cam_y).
    player_set_pos(10, 6); 
        
    update_fog_player_sprite_pos();
}

void try_spawn_neighbor(uint8_t target_slot, uint8_t required_connection) {
    uint8_t tid = fast_rand() % 21;
    
    // Get the structure for this potential room
    RoomState potential = get_room_state_from_template(tid);

    // Check if it has the connector we need
    if (potential.flags & required_connection) {
        // It fits! Copy the validated structure into the grid
        dungeon_graph[target_slot] = potential;
    } else {
        // It's a dead end, maybe try a different template or place a wall
    }
}

uint8_t find_room_template(uint8_t connecting_template_id, uint8_t must_have, uint8_t must_not) {
    uint8_t candidates[21];
    uint8_t count = 0;
    
    for (uint8_t i = 0; i < 21; i++) {
        if(i == connecting_template_id) continue; // Skip the same template
        RoomState temp = get_room_state_from_template(i);
        
        // 1. It must have all connectors leading to existing neighbors
        if ((temp.flags & must_have) != must_have) continue;
        
        // 2. It must NOT have connectors leading into solid walls of neighbors
        if ((temp.flags & must_not)) continue;

        candidates[count++] = i;
    }
    
    if (count == 0) return 0xFF; // No perfect fit found
    return candidates[fast_rand() % count];
}

void spawn_entire_dungeon(void) {
    uint8_t changed = 1;
    while (changed) {
        changed = 0;
        for (uint8_t idx = 0; idx < 9; idx++) {
            if (dungeon_graph[idx].template_id == 0xFF) {
                uint8_t gx = idx % 3;
                uint8_t gy = idx / 3;
                uint8_t must_have = 0;
                uint8_t must_not = 0;

                // --- NORTH (Wrap to Bottom Row) ---
                // If gy is 0, neighbor is (gx, 2) which is idx + 6
                uint8_t n_idx = (gy > 0) ? (idx - 3) : (idx + 6);
                if (dungeon_graph[n_idx].template_id != 0xFF) {
                    if (dungeon_graph[n_idx].flags & CONN_S) must_have |= CONN_N;
                    else must_not |= CONN_N;
                }

                // --- SOUTH (Wrap to Top Row) ---
                // If gy is 2, neighbor is (gx, 0) which is idx - 6
                uint8_t s_idx = (gy < 2) ? (idx + 3) : (idx - 6);
                if (dungeon_graph[s_idx].template_id != 0xFF) {
                    if (dungeon_graph[s_idx].flags & CONN_N) must_have |= CONN_S;
                    else must_not |= CONN_S;
                }

                // --- WEST (Wrap to Right Column) ---
                // If gx is 0, neighbor is (2, gy) which is idx + 2
                uint8_t w_idx = (gx > 0) ? (idx - 1) : (idx + 2);
                if (dungeon_graph[w_idx].template_id != 0xFF) {
                    if (dungeon_graph[w_idx].flags & CONN_E) must_have |= CONN_W;
                    else must_not |= CONN_W;
                }

                // --- EAST (Wrap to Left Column) ---
                // If gx is 2, neighbor is (0, gy) which is idx - 2
                uint8_t e_idx = (gx < 2) ? (idx + 1) : (idx - 2);
                if (dungeon_graph[e_idx].template_id != 0xFF) {
                    if (dungeon_graph[e_idx].flags & CONN_W) must_have |= CONN_E;
                    else must_not |= CONN_E;
                }

                if (must_have > 0) {
                    uint8_t tid = find_room_template(dungeon_graph[idx].template_id, must_have, must_not);
                    if (tid != 0xFF) {
                        dungeon_graph[idx] = get_room_state_from_template(tid);
                        changed = 1;
                    }
                }
            }
        }
    }
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
    uint8_t random_template = fast_rand() % TOTAL_TEMPLATES; 

    // 3. Place it in the center slot (Index 4 for 3x3)
    dungeon_graph[4].template_id = random_template;

    // 4. Sync the room logic
    sync_room_connectors(4);
    find_player_spawn(4);    
    update_camera();     
    sync_player_to_view();

    // 5. Spawn the rest of the dungeon
    spawn_entire_dungeon();
    
    // 6. Initialize hardware smooth scrolling and do first screen redraw
    scroll_x_pixels = 0;
    scroll_y_pixels = 0;
    apply_hardware_scroll();
    needs_screen_redraw = 1;
}

uint8_t get_tile_at_scaled(uint8_t tx, uint8_t ty, uint8_t gx, uint8_t gy, uint8_t lx, uint8_t ly) {
    RoomState *r = &dungeon_graph[(gy * GRID_SIZE) + gx];
    
    // Skip empty or uninitialized rooms, but render all revealed rooms
    if (r->template_id == 0xFF) return TILE_WALL;

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
    // Simple approach: redraw entire screen when a tile boundary is crossed
    // Optimized coordinate calculations to avoid expensive operations
    if (!needs_screen_redraw) {
        return;
    }
    
    needs_screen_redraw = 0;
    
    uint8_t screen_x, screen_y;
    uint8_t local_cam_x = cam_x;
    uint8_t local_cam_y = cam_y;
    
    for (screen_y = 0; screen_y < 12; screen_y++) {
        for (screen_x = 0; screen_x < 20; screen_x++) {
            // Use lookup tables or direct calculation instead of modulo
            uint8_t world_tx = local_cam_x + screen_x;
            if (world_tx >= 33) world_tx -= 33;
            
            uint8_t world_ty = local_cam_y + screen_y;
            if (world_ty >= 33) world_ty -= 33;

            // Inline division by 11 using lookup or bit operations
            uint8_t gx, gy, lx, ly;
            
            // gx = world_tx / 11
            if (world_tx < 11) gx = 0;
            else if (world_tx < 22) gx = 1;
            else gx = 2;
            
            // gy = world_ty / 11
            if (world_ty < 11) gy = 0;
            else if (world_ty < 22) gy = 1;
            else gy = 2;
            
            // lx = world_tx % 11
            lx = world_tx - (gx * 11);
            
            // ly = world_ty % 11
            ly = world_ty - (gy * 11);

            uint8_t tile = get_tile_at_scaled(world_tx, world_ty, gx, gy, lx, ly);
            draw_dungeon_tile(screen_x, screen_y, tile);
        }
    }
}

void update_fog_player_sprite_pos(void) {
    // Force coordinates to the center of a standard C64 screen
    // X = 160 + 24 (border) = 184
    // Y = 100 + 50 (border) - 3 (your offset fix) = 147
    unsigned int sx = 184; 
    unsigned char sy = 147; 

    // 1. Set Y position
    vic.spr_pos[0].y = sy;

    // 2. Set X position (Low Byte)
    vic.spr_pos[0].x = (unsigned char)(sx & 0xFF);

    // 3. CRITICAL: Handle the Most Significant Bit (MSB)
    // If we don't clear this, and it was set previously, 
    // the sprite will be 256 pixels to the right (off-screen).
    if (sx > 255) {
        vic.spr_msbx |= 0x01;  // Set bit 0 for Sprite 0
    } else {
        vic.spr_msbx &= ~0x01; // Clear bit 0 for Sprite 0
    }
}
