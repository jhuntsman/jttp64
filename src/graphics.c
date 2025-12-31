#include <c64/vic.h>
#include <conio.h>
#include <string.h>
#include "assets.h"
#include "graphics.h"

void init_graphics() {
    vic.ctrl2 &= ~0x10;    // Force High-Resolution Text Mode
    vic.color_back = COLOR_LT_GREEN;   // Light Green
    vic.color_border = 0;  // Black
}

void init_charset() {
    // Set Charset at $2000
    memcpy((unsigned char*)CHARSET_DEST, charset_data, 2048);
    *((unsigned char*)0xD018) = (*((unsigned char*)0xD018) & 0xF1) | 0x08;
}

void wait_vblank() {
    while (vic.raster != 0xff);  // Wait for the beam to hit the bottom
}
// Player sprite functions moved to src/sprite.c