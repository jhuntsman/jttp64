#ifndef JTTP64_DUNGEON_H
#define JTTP64_DUNGEON_H

void init_dungeon(void);
void render_dungeon(void);
void debug_view_meta_room(void);

// Accessor to read live dungeon tile at (x,y) (0..63). Returns 0 if out of range.
unsigned char dungeon_get_tile(unsigned char x, unsigned char y);

// Get camera/top-left tile for the dungeon viewport (0..63)
void dungeon_get_camera(unsigned char *cx, unsigned char *cy);

// Collision helper for fog/dungeon tiles
unsigned char is_crawlable(unsigned char x, unsigned char y, unsigned char tile_id);

// Helper: get tile using screen-relative viewport coords (tx,ty) where
// tx in [0..VIEW_W), ty in [0..VIEW_H). Returns same as dungeon_get_tile.
unsigned char dungeon_get_tile_screen(unsigned char tx, unsigned char ty);

#endif  // JTTP64_DUNGEON_H
