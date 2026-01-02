#include <string.h>
#include <stdint.h>
#include <conio.h>
#include "assets.h"
#include "graphics.h"
#include "utils.h"
#include "player.h"
/* Use the overworld meta-tile drawer for a sanity check. */
extern void draw_meta_tile(unsigned char tx, unsigned char ty, unsigned char tile_id);

// Screen Memory for C64
#define TILE_FLOOR  0x24
#define TILE_CONNECTOR 0x25
#define TILE_REVEAL 0x26
#define TILE_SPAWN  0x27
#define TILE_WALL 0x00
#define TILE_BLACK  0x00

#define MAP_WIDTH 64
#define MAP_HEIGHT 64
#define ROOM_SIZE 9

#include <stddef.h>

// Precomputed room index and candidate lists kept static at file scope
typedef struct { uint8_t flags; int8_t sx, sy; } RoomInfo;
static RoomInfo room_index[21];
static uint8_t room_index_ready = 0;
static uint8_t candidates_n[21]; static uint8_t candn = 0;
static uint8_t candidates_s[21]; static uint8_t cands = 0;
static uint8_t candidates_w[21]; static uint8_t candw = 0;
static uint8_t candidates_e[21]; static uint8_t cande = 0;
static uint8_t candidates_sparse[21]; static uint8_t candsparse = 0;

/* live_dungeon_map must live in writable BSS, not the registry section where
    `dungeon_room_data` (const) is linked. Placing both in `registry` can cause
    them to overlap at link time. Allocate `live_dungeon_map` in normal data. */
static unsigned char live_dungeon_map[4096];

// Fog mask in BSS (Main RAM)
static uint8_t fog_mask[512];

// Made these non-static so your renderer/player code can see them!
uint8_t player_x, player_y;
uint8_t cam_x, cam_y;

// Viewport size in meta-tiles (each meta-tile is 2x2 screen chars)
#define VIEW_W 20
#define VIEW_H 12

// forward-declare fog check used by renderer
static unsigned char dungeon_is_revealed_world(unsigned char wx, unsigned char wy);

void reveal_room_area(uint8_t start_x, uint8_t start_y) {
    // Compute the aligned top-left of the room that contains (start_x,start_y).
    // This allows callers to pass any tile inside a room (e.g. player pos)
    // and have the entire ROOM_SIZE x ROOM_SIZE area revealed.
    uint8_t room_x = start_x - (start_x % ROOM_SIZE);
    uint8_t room_y = start_y - (start_y % ROOM_SIZE);

    for (uint8_t ry = 0; ry < ROOM_SIZE; ry++) {
        for (uint8_t rx = 0; rx < ROOM_SIZE; rx++) {
            // Apply 64x64 wrapping to the reveal coordinates
            uint8_t wrapped_x = (room_x + rx) & 63;
            uint8_t wrapped_y = (room_y + ry) & 63;
            uint16_t map_idx = (wrapped_y << 6) + wrapped_x;

            fog_mask[map_idx >> 3] |= (1 << (map_idx & 7));
        }
    }
}

void init_dungeon(void) {
    // Generate an 8x8 dungeon of ROOM_SIZE x ROOM_SIZE rooms by selecting
    // compatible rooms from the embedded `dungeon_room_data`.
    const uint8_t src_cols = 63 / ROOM_SIZE; // 7
    const uint8_t src_rows = 27 / ROOM_SIZE; // 3
    const uint8_t room_count = src_cols * src_rows; // 21

    // Use precomputed room index (flags + spawn) to speed up generation.
    // File-scope statics `room_index` and `candidates_*` are declared above.

    if (!room_index_ready) {
        for (uint8_t rid = 0; rid < room_count; ++rid) {
            room_index[rid].flags = 0;
            room_index[rid].sx = -1;
            room_index[rid].sy = -1;
            uint8_t base_col = (rid % src_cols) * ROOM_SIZE;
            uint8_t base_row = (rid / src_cols) * ROOM_SIZE;

            // fixed positions: north=(5,0), south=(5,8), west=(0,5), east=(8,5)
            uint16_t sx_n = base_col + 5;
            uint16_t sy_n = base_row + 0;
            uint16_t sidx_n = (sy_n * (src_cols * ROOM_SIZE)) + sx_n;
            if ((dungeon_room_data[sidx_n] & 0x3F) == TILE_CONNECTOR) room_index[rid].flags |= 1<<0;

            uint16_t sx_s = base_col + 5;
            uint16_t sy_s = base_row + (ROOM_SIZE - 1);
            uint16_t sidx_s = (sy_s * (src_cols * ROOM_SIZE)) + sx_s;
            if ((dungeon_room_data[sidx_s] & 0x3F) == TILE_CONNECTOR) room_index[rid].flags |= 1<<1;

            uint16_t sx_w = base_col + 0;
            uint16_t sy_w = base_row + 5;
            uint16_t sidx_w = (sy_w * (src_cols * ROOM_SIZE)) + sx_w;
            if ((dungeon_room_data[sidx_w] & 0x3F) == TILE_CONNECTOR) room_index[rid].flags |= 1<<2;

            uint16_t sx_e = base_col + (ROOM_SIZE - 1);
            uint16_t sy_e = base_row + 5;
            uint16_t sidx_e = (sy_e * (src_cols * ROOM_SIZE)) + sx_e;
            if ((dungeon_room_data[sidx_e] & 0x3F) == TILE_CONNECTOR) room_index[rid].flags |= 1<<3;

            // find spawn within room (if any)
            for (uint8_t ry = 0; ry < ROOM_SIZE; ++ry) {
                for (uint8_t rx = 0; rx < ROOM_SIZE; ++rx) {
                    uint16_t sx = base_col + rx;
                    uint16_t sy = base_row + ry;
                    uint16_t sidx = (sy * (src_cols * ROOM_SIZE)) + sx;
                    unsigned char t = dungeon_room_data[sidx] & 0x3F;
                    if (t == TILE_SPAWN) {
                        room_index[rid].sx = rx;
                        room_index[rid].sy = ry;
                    }
                }
            }
        }
        // build candidate lists for each opposite connector
        candn = cands = candw = cande = candsparse = 0;
        for (uint8_t r = 0; r < room_count; ++r) {
            if (room_index[r].flags & (1<<0)) candidates_n[candn++] = (uint8_t)r;
            if (room_index[r].flags & (1<<1)) candidates_s[cands++] = (uint8_t)r;
            if (room_index[r].flags & (1<<2)) candidates_w[candw++] = (uint8_t)r;
            if (room_index[r].flags & (1<<3)) candidates_e[cande++] = (uint8_t)r;
            // sparse 'any' list: deterministic ~1/4 of rooms
            if ((r & 3) == 0) candidates_sparse[candsparse++] = (uint8_t)r;
        }
        room_index_ready = 1;
    }

    // Live room grid: use 8x8 and support wrap-around copying into the 64x64 map.
    // When a room row crosses the 64-column boundary we perform a split memcpy.
    #define LIVE_GRID 8
    int8_t grid[LIVE_GRID][LIVE_GRID];
    for (int y = 0; y < LIVE_GRID; ++y) for (int x = 0; x < LIVE_GRID; ++x) grid[y][x] = -1;

    // Choose seed room for (0,0): prefer a room with a spawn
    int seed = -1;
    for (uint8_t r = 0; r < room_count; ++r) if (room_index[r].sx >= 0) seed = r;
    if (seed < 0) seed = fast_rand() % room_count;
    grid[0][0] = seed;

    // BFS queue for expansion
    typedef struct { int8_t x,y; } Cell;
    Cell queue[LIVE_GRID * LIVE_GRID]; int qh=0, qt=0;
    queue[qt++] = (Cell){0,0};

    while (qh < qt) {
        Cell c = queue[qh++];
        int cx = c.x, cy = c.y;
        int cur_rid = grid[cy][cx];
        // for each neighbor direction
        const int dx[4] = {0, 1, 0, -1};
        const int dy[4] = {-1, 0, 1, 0};
        for (int dir = 0; dir < 4; ++dir) {
            int nx = cx + dx[dir];
            int ny = cy + dy[dir];
            int wx = nx;
            int wy = ny;
            int wrapped = 0;
            if (nx < 0) { wx = nx + LIVE_GRID; wrapped = 1; }
            else if (nx >= LIVE_GRID) { wx = nx - LIVE_GRID; wrapped = 1; }
            if (ny < 0) { wy = ny + LIVE_GRID; wrapped = 1; }
            else if (ny >= LIVE_GRID) { wy = ny - LIVE_GRID; wrapped = 1; }
            if (grid[wy][wx] != -1) continue; // already placed

                // determine whether current has connector in this direction
                uint8_t required = 0;
                if (dir == 0) required = (room_index[cur_rid].flags & (1<<0)) != 0;
                else if (dir == 2) required = (room_index[cur_rid].flags & (1<<1)) != 0;
                else if (dir == 1) required = (room_index[cur_rid].flags & (1<<3)) != 0;
                else required = (room_index[cur_rid].flags & (1<<2)) != 0;

                // find candidates that have opposite connector
                uint8_t *candidates = NULL; int candc = 0;
                if (!required) {
                    candidates = candidates_sparse;
                    candc = candsparse;
                } else {
                    if (dir == 0) { candidates = candidates_s; candc = cands; }
                    else if (dir == 2) { candidates = candidates_n; candc = candn; }
                    else if (dir == 1) { candidates = candidates_w; candc = candw; }
                    else { candidates = candidates_e; candc = cande; }
                }

            if (candc == 0) {
                // leave empty (wall)
                grid[wy][wx] = -1;
            } else {
                int pick = candidates[fast_rand() % candc];
                grid[wy][wx] = pick;
                queue[qt++] = (Cell){(int8_t)wx, (int8_t)wy};
            }
        }
    }

    // Copy selected rooms into live_dungeon_map (64x64)
    // Copy selected rooms into live_dungeon_map (64x64).
    // Use per-row memcpy/memset for much faster copying; split copies when the
    // destination row would wrap past column 63 (index 63 -> width 64).
    for (int ry = 0; ry < LIVE_GRID; ++ry) {
        for (int rx = 0; rx < LIVE_GRID; ++rx) {
            int8_t rid = grid[ry][rx];
            for (int y = 0; y < ROOM_SIZE; ++y) {
                uint16_t map_x = (rx * ROOM_SIZE);
                uint16_t map_y = (ry * ROOM_SIZE) + y;
                uint16_t midx = (map_y << 6) + map_x; // start index in live_dungeon_map

                if (rid < 0) {
                    // empty room row -> fill ROOM_SIZE tiles with wall
                    if (map_x + ROOM_SIZE <= 64) {
                        memset(&live_dungeon_map[midx], TILE_WALL, ROOM_SIZE);
                    } else {
                        int tail = 64 - map_x;
                        memset(&live_dungeon_map[midx], TILE_WALL, tail);
                        memset(&live_dungeon_map[(map_y << 6)], TILE_WALL, ROOM_SIZE - tail);
                    }
                } else {
                    uint8_t base_col = (rid % src_cols) * ROOM_SIZE;
                    uint8_t base_row = (rid / src_cols) * ROOM_SIZE;
                    uint16_t sx = base_col;
                    uint16_t sy = base_row + y;
                    uint16_t sidx = (sy * (src_cols * ROOM_SIZE)) + sx;
                    if (map_x + ROOM_SIZE <= 64) {
                        memcpy(&live_dungeon_map[midx], &dungeon_room_data[sidx], ROOM_SIZE);
                    } else {
                        int tail = 64 - map_x;
                        // copy tail to end of row
                        memcpy(&live_dungeon_map[midx], &dungeon_room_data[sidx], tail);
                        // copy remaining bytes to start of same row
                        memcpy(&live_dungeon_map[(map_y << 6)], &dungeon_room_data[sidx + tail], ROOM_SIZE - tail);
                    }
                }
            }
        }
    }

    // Place player at spawn within the seed room (0,0) if present
    int8_t s = grid[0][0];
    if (s >= 0 && room_index[s].sx >= 0) {
        player_x = (0 * ROOM_SIZE) + room_index[s].sx;
        player_y = (0 * ROOM_SIZE) + room_index[s].sy;
    } else {
        // fallback center
        player_x = ROOM_SIZE/2;
        player_y = ROOM_SIZE/2;
    }

    // Apply to player API so game systems use the spawn position
    player_set_pos(player_x, player_y);

    // clear fog and reveal starting room (center on the chosen spawn)
    memset(fog_mask, 0, 512);
    reveal_room_area(player_x, player_y);

    // initialize camera centered on player spawn (left-of-center for even sizes)
    cam_x = (uint8_t)((player_x - ((VIEW_W - 1) / 2)) & 63);
    cam_y = (uint8_t)((player_y - ((VIEW_H - 1) / 2)) & 63);
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

void render_dungeon(void) {
    // Center camera on player (tile coords) then render a VIEW_W x VIEW_H
    // window reading from the 64x64 `live_dungeon_map`. Wrap at 64.
    uint8_t px = player_get_x();
    uint8_t py = player_get_y();

    // center camera so player is in the middle; for even view sizes prefer
    // the left-of-center tile so visual centering matches sprite anchor.
    cam_x = (uint8_t)((px - ((VIEW_W - 1) / 2)) & 63);
    cam_y = (uint8_t)((py - ((VIEW_H - 1) / 2)) & 63);

    for (int ty = 0; ty < VIEW_H; ++ty) {
        for (int tx = 0; tx < VIEW_W; ++tx) {
            uint8_t sx = (uint8_t)((cam_x + tx) & 63);
            uint8_t sy = (uint8_t)((cam_y + ty) & 63);
            unsigned char tile = dungeon_get_tile(sx, sy) & 0x3F;
            unsigned char draw_tile;
            if (dungeon_is_revealed_world(sx, sy)) {
                draw_tile = tile;
                //if (tile == TILE_SPAWN || tile == TILE_REVEAL || tile == TILE_CONNECTOR) draw_tile = TILE_FLOOR;
            } else {
                //draw_tile = TILE_BLACK;
            }
            draw_meta_tile((unsigned char)tx, (unsigned char)ty, draw_tile);
        }
    }    
}

void dungeon_get_camera(unsigned char *cx, unsigned char *cy) {
    if (cx) *cx = cam_x;
    if (cy) *cy = cam_y;
}

unsigned char dungeon_get_tile(unsigned char x, unsigned char y) {
    // bounds clamp
    x &= 63;
    y &= 63;
    uint16_t idx = ((uint16_t)y << 6) + x;
    return live_dungeon_map[idx];
}

unsigned char dungeon_get_tile_screen(unsigned char tx, unsigned char ty) {
    // Translate from viewport/screen-relative tile coords to world 0..63 coords
    uint8_t wx = (uint8_t)((cam_x + tx) & 63);
    uint8_t wy = (uint8_t)((cam_y + ty) & 63);
    return dungeon_get_tile(wx, wy);
}

// Return non-zero if the given world tile (0..63 coords) is revealed in fog
static unsigned char dungeon_is_revealed_world(unsigned char wx, unsigned char wy) {
    uint16_t map_idx = ((uint16_t)wy << 6) + wx;
    uint16_t byte = map_idx >> 3;
    uint8_t bit = 1 << (map_idx & 7);
    return (fog_mask[byte] & bit) != 0;
}

// Collision helper for fog/dungeon tiles: returns non-zero if the tile is
// crawlable. Uses dungeon tile constants defined in this file.
unsigned char is_crawlable(unsigned char x, unsigned char y, unsigned char tile_id) {
    (void)x; (void)y;
    unsigned char t = tile_id & 0x3F;
    return t != TILE_WALL;
}