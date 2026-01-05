#ifndef JTTP64_DUNGEON_H
#define JTTP64_DUNGEON_H

void init_dungeon(void);
void render_dungeon(void);
unsigned char is_crawlable(unsigned char x, unsigned char y, unsigned char tile_id);

#endif  // JTTP64_DUNGEON_H
