#include <c64/vic.h>
#include <conio.h>
#include <string.h>

#include "graphics.h"
#include "sprite.h"
#include "assets.h"
#include "player.h"
#include "globals.h"
#include "dungeon.h"

static unsigned char pulse_clock = 0;

void handle_player_pulse(void) {
    pulse_clock++;

    if ((pulse_clock & 0x3F) > 0x38) {
        vic.spr_enable &= 0xFE; // Hide Sprite 0
    } else {
        vic.spr_enable |= 0x01; // Show Sprite 0
    }
}

void init_player_sprite(void) {
    unsigned char* s_mem = (unsigned char*)(SPRITE_BLOCK * 64);
    memset(s_mem, 0, 64);

    // Build the sprite from the 4 characters that make up Tile $01
    // Tile $01 = Chars: TopLeft(02), TopRight(03), BottomLeft(01),
    // BottomRight(01)
    for (unsigned char i = 0; i < 8; i++) {
        s_mem[i * 3] = charset_data[0x02 * 8 + i];
        s_mem[i * 3 + 1] = charset_data[0x03 * 8 + i];
        s_mem[(i + 8) * 3] = charset_data[0x01 * 8 + i];
        s_mem[(i + 8) * 3 + 1] = charset_data[0x01 * 8 + i];
    }

    ((unsigned char*)SPRITE_PTR)[0] = SPRITE_BLOCK;
    vic.spr_enable = 0x01;
    vic.spr_color[0] = COLOR_WHITE;
}

void update_player_sprite_pos(void) {
    unsigned char relx = player_get_x();
    unsigned char rely = player_get_y();
    
    const unsigned int sx = 24 + (relx * 16);
    const unsigned char sy = 50 + (rely * 16);

    vic.spr_pos[0].x = (unsigned char)(sx & 0xFF);
    vic.spr_pos[0].y = sy;

    if (sx & 0x100)
        vic.spr_msbx |= 0x01;
    else
        vic.spr_msbx &= ~0x01;
}
