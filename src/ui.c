#include <c64/vic.h>
#include <c64/sid.h>
#include <conio.h>
#include "globals.h"
#include "graphics.h"
#include "ui.h"

static unsigned char border_timer = 0;
static unsigned char border_color = 0;

void ui_trigger_border(unsigned char duration, unsigned char color) {
    border_timer = duration;
    border_color = color;
    vic.color_border = color;
}

void ui_update(void) {
    if (border_timer > 0) {
        if (--border_timer == 0) vic.color_border = COLOR_BLACK;
    }
}

// Snore logic moved to src/snore.c

void draw_hud(void) {
    for (unsigned int i = HUD_START; i < 1000; i++) {
        SCREEN_RAM[i] = SOLID_BLOCK;
        COLOR_RAM[i] = COLOR_HUD;
    }
}
