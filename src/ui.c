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

/* cached HUD state to avoid unnecessary writes */
static int prev_health = -1;
static int prev_arrows = -1;
static int prev_level = -1;
static xp_t prev_xp = (xp_t)-1;

/* inventory mask: bit0=axe, bit1=key, bit2=boat, bit3=crown1, bit4=crown2 */
static unsigned char prev_inventory = 0xFF;

/* helper to write a 2-digit number using the HUD charset */
// removed custom hud_write_2digit in favor of existing draw_num_2digit()

void ui_trigger_border(unsigned char duration, unsigned char color) {
    border_timer = duration;
    border_color = color;
    vic.color_border = color;
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
    if(!player_has_axe()
       && !player_has_key()
       && !player_has_boat()
       && !player_has_crown_1()
       && !player_has_crown_2()) {
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
    
    // level (restore original position) - display is 1-based for player-facing UI
    {
        int disp_level = player_get_level() + 1;
        if (disp_level > MAX_LEVEL) disp_level = MAX_LEVEL;
        draw_num_2digit(37, 23, (uint8_t)disp_level);
    }

    // Experience (XP) label reserved on the line below; initial value drawn in hud_update
    draw_hud_text(29, 24, "XP:");
    
    
    draw_hud_raw(0, 24, "\xC2");    
    draw_hud_raw(31, 22, "\xC3\xC3\xC3\xC3\xC3\xC3\xC3\xC3\xEE");
    draw_hud_raw(39, 23, "\xC2");
    draw_hud_raw(39, 24, "\xC2");
}

void hud_update(void) {
    int health = player_get_health();
    int arrows = player_get_arrows();
    int level = player_get_level();
    int disp_level = level + 1;
    if (disp_level > MAX_LEVEL) disp_level = MAX_LEVEL;
    
    if (health != prev_health) {
        for (int i = 0; i < health; ++i) {
            if (i < health) {
                draw_hud_raw(4 + i, 23, "\xD3"); /* heart icon (raw) */
            } else {
                draw_hud_text(4 + i, 23, " ");
            }
        }
        prev_health = health;
    }

    /* Update arrows (two-digit) at x=10,y=23 */
    if (arrows != prev_arrows) {
        draw_num_2digit(10, 23, (uint8_t)arrows);
        prev_arrows = arrows;
    }

    /* Update inventory area (x=20..27,y=23) */
    unsigned char inv_mask = 0;
    if (player_has_axe()) inv_mask |= 1 << 0;
    if (player_has_key()) inv_mask |= 1 << 1;
    if (player_has_boat()) inv_mask |= 1 << 2;
    if (player_has_crown_1()) inv_mask |= 1 << 3;
    if (player_has_crown_2()) inv_mask |= 1 << 4;

    if (inv_mask != prev_inventory) {
        const uint16_t inv_pos = (23 * 40) + 20;
        /* clear area to HUD background (use draw_hud_text to get proper charset offset) */
        for (int i = 0; i < 8; ++i) draw_hud_text(20 + i, 23, " ");

        if (inv_mask == 0) {
            draw_hud_text(20, 23, "EMPTY");
        } else {
            int x = 20;
            if (inv_mask & (1 << 0)) { draw_hud_raw(x, 23, "\x1F"); x++; }
            if (inv_mask & (1 << 1)) { draw_hud_raw(x, 23, "\x2F"); x++; }
            if (inv_mask & (1 << 2)) { draw_hud_raw(x, 23, "\x3F"); x++; }
            if (inv_mask & (1 << 3)) { draw_hud_raw(x, 23, "\x5E"); x++; }
            if (inv_mask & (1 << 4)) { draw_hud_raw(x, 23, "\x5F"); x++; }
        }
        prev_inventory = inv_mask;
    }

    /* Update level at x=37,y=23 (displayed as 1-based) */
    if (disp_level != prev_level) {
        draw_num_2digit(37, 23, (uint8_t)disp_level);
        prev_level = disp_level;
    }

    /* Update XP at x=32..38 on row 24, width=7 */
    xp_t xp = player_get_xp();
    if (xp != prev_xp) {
        char buf[8];
        int width = 7;
        for (int i = 0; i < width; ++i) buf[i] = ' ';
        buf[width] = '\0';
        xp_t tmp = xp;
        int pos = width - 1;
        if (tmp == 0) {
            buf[pos] = '0';
        } else {
            while (pos >= 0 && tmp > 0) {
                buf[pos] = '0' + (tmp % 10);
                tmp /= 10;
                pos--;
            }
        }
        draw_hud_text(32, 24, buf);
        prev_xp = xp;
    }
}

void ui_update(void) {
    if (border_timer > 0) {
        if (--border_timer == 0) vic.color_border = COLOR_BLACK;
    }
    /* update dynamic HUD elements every frame (called only in MODE_MAP) */
    hud_update();
}
