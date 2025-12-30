#ifndef JTTP_GRAPHICS_H
#define JTTP_GRAPHICS_H

#define SCREEN_RAM ((unsigned char*)0x0400)
#define COLOR_RAM ((unsigned char*)0xD800)
#define CHARSET_DEST 0x2000

#define SPRITE_PTR 0x07F8
#define SPRITE_BLOCK 13

void init_graphics();
void init_charset();

void wait_vblank();


#endif  // JTTP_GRAPHICS_H
