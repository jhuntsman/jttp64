#include <c64/vic.h>
#include <c64/sid.h>
#include <conio.h>
#include <stdint.h>
#include <stdio.h>
#include "globals.h"
#include "graphics.h"
#include "ui.h"
#include "player.h"

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


void draw_hud_text(const uint8_t x, const uint8_t y, const char* text) {
    // Calculate the screen position (y is row 0-24, x is col 0-39)
    const uint16_t pos = (y * 40) + x;

    for (int i = 0; text[i] != '\0'; i++) {
        uint8_t c = text[i];

        // 1. Convert C's ASCII to C64 Screen Codes
        // Most compilers handle 'A'-'Z' as 65-90.
        // In the C64 ROM font, 'A' is index 1.
        if (c >= 'A' && c <= 'Z') {
            c = c - 64;
        } else if (c >= 'a' && c <= 'z') {
            c = c - 96;
        } else if (c >= '0' && c <= '9') {
            c = c; // Numbers usually match up
        } else if (c == ':') {
            c = 58; // Colon in ROM font
        } else if (c == ' ') {
            c = 32; // Space
        }

        // 2. Add 128 to jump to the second half of your hybrid charset
        SCREEN_RAM[pos + i] = c + HUD_CHARSET_OFFSET; 
        //COLOR_RAM[pos + i] = COLOR_HUD;       
    }
}

void draw_hud_raw(const uint8_t x, const uint8_t y, const char* text) {
    // Calculate the screen position (y is row 0-24, x is col 0-39)
    const uint16_t pos = (y * 40) + x;

    for (int i = 0; text[i] != '\0'; i++) {
        uint8_t c = text[i];

        // 1. Convert C's ASCII to C64 Screen Codes
        // Most compilers handle 'A'-'Z' as 65-90.
        // In the C64 ROM font, 'A' is index 1.
        if (c >= 'A' && c <= 'Z') {
            c = c - 64;
        } else if (c >= 'a' && c <= 'z') {
            c = c - 96;
        } else if (c >= '0' && c <= '9') {
            c = c; // Numbers usually match up
        }

        // 2. Add 128 to jump to the second half of your hybrid charset
        SCREEN_RAM[pos + i] = c; 
        //COLOR_RAM[pos + i] = COLOR_HUD;       
    }
}

void draw_num_2digit(const uint8_t x, const uint8_t y, const uint8_t val) {
    const uint16_t pos = (y * 40) + x;

    // Standard ROM font numbers '0'-'9' are indices 48-57
    // We add 128 because your font is in the second half of the charset
    const uint8_t font_offset = 128 + 48;

    // Tens Digit
    // (val / 10) gets the first digit. 
    // Example: 25 / 10 = 2.
    SCREEN_RAM[pos] = (val / 10) + font_offset;

    // Units Digit
    // (val % 10) gets the remainder. 
    // Example: 25 % 10 = 5.
    SCREEN_RAM[pos + 1] = (val % 10) + font_offset;
}

void draw_hud(void) {
    for (unsigned int i = HUD_START; i < 1000; i++) {
        SCREEN_RAM[i] = 0xA0;  // Solid Block Character
        COLOR_RAM[i] = COLOR_HUD;
    }

    draw_hud_text(8, 22, "JOURNEY TO THUNDER PEAK");
    draw_hud_raw(0, 22, "\xF0\xC3\xC3\xC3\xC3\xC3\xC3\xC3");
    draw_hud_raw(0, 23, "\xC2");
    
    // HUD Labels
    draw_hud_text(1, 23, "HP:     :     PACK:             LVL:");

    // Heart icons for hit points
    for(int i=0; i < player_get_health(); i++) {
        draw_hud_raw(4+ i, 23, "\xD3");  
    }
    // arrows
    draw_hud_raw(8, 23, "\x0F"); // Quiver icon        
    draw_num_2digit(10, 23, player_get_arrows());

    // inventory
    if(!player_has_axe()) {
        // empty inventory
        draw_hud_text(20, 23, "EMPTY");    
    }
    else {
        unsigned char inventory_x = 20;
        if(player_has_axe()) {
            draw_hud_raw(inventory_x, 23, "\x1F"); // Axe icon
            inventory_x += 1;
        }
        if(player_has_key()) {
            draw_hud_raw(inventory_x, 23, "\x2F"); // Key icon
            inventory_x += 1;
        }
        if(player_has_boat()) {
            draw_hud_raw(inventory_x, 23, "\x3F"); // Boat icon
            inventory_x += 1;
        }
        if(player_has_crown_1()) {
            draw_hud_raw(inventory_x, 23, "\x5E"); // Crown 1 icon
            inventory_x += 1;
        }
        if(player_has_crown_2()) {
            draw_hud_raw(inventory_x, 23, "\x5F"); // Crown 2 icon
            inventory_x += 1;
        }
    }
    
    // level
    draw_num_2digit(37, 23, player_get_level());
    
    
    draw_hud_raw(0, 24, "\xC2");    
    draw_hud_raw(31, 22, "\xC3\xC3\xC3\xC3\xC3\xC3\xC3\xC3\xEE");
    draw_hud_raw(39, 23, "\xC2");
    draw_hud_raw(39, 24, "\xC2");
}
