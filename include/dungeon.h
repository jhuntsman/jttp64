#ifndef JTTP64_DUNGEON_H
#define JTTP64_DUNGEON_H

// tile types
#define TILE_FLOOR  0x24
#define TILE_CONNECTOR 0x25
#define TILE_REVEAL 0x26
#define TILE_SPAWN  0x27
#define TILE_WALL 0x00
#define TILE_BLACK  0x00

void init_dungeon(void);
void render_dungeon(void);
void update_camera(void);
void apply_hardware_scroll(void);
void redraw_column(uint8_t screen_col);
void redraw_row(uint8_t screen_row);
void scroll_viewport_left(void);
void scroll_viewport_right(void);
void scroll_viewport_up(void);
void scroll_viewport_down(void);
unsigned char is_crawlable(unsigned char x, unsigned char y, unsigned char tile_id);
uint8_t get_tile_at_scaled(uint8_t tx, uint8_t ty, uint8_t gx, uint8_t gy, uint8_t lx, uint8_t ly);

void update_fog_player_sprite_pos(void);

#endif  // JTTP64_DUNGEON_H
