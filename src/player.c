#include <conio.h>
#include "player.h"
#include "ui.h"
#include "input.h"

/* forward declare level-up sound (implemented in sound.c) */
extern void play_levelup(void);

// Player Position
static unsigned char p_x = 0;
static unsigned char p_y = 0;

// Packed player stats to save memory on C64
// bits 7-4 : lives (4 bits)
// bits 3-0 : health (4 bits)
static unsigned char player_stats = 0;

// Separate small fields we need directly
static unsigned char player_level = 0;
static unsigned char player_arrows = 0;

// Experience/Score (allow up to millions) - use 32-bit unsigned
static xp_t player_xp = 0;

// Inventory packed into single byte to save space (bit flags)
// uses PLAYER_INV_* defines from include/player.h
static unsigned char player_inventory = 0;


void init_player(void) {
    // starting lives
    player_stats = ((MAX_LIVES << PLAYER_LIVES_SHIFT) & PLAYER_LIVES_MASK) | (3 & PLAYER_HEALTH_MASK);

    // starting level (begin at 0 so XP thresholds behave as cumulative totals)
    player_level = 0;

    // starting arrows
    player_arrows = 3;
    if (player_arrows > MAX_ARROWS) player_arrows = MAX_ARROWS;

    // clear inventory
    player_inventory = 0;
}

void reset_player(void) {
    // reset after death
    player_stats = (player_stats & PLAYER_LIVES_MASK) | (3 & PLAYER_HEALTH_MASK);

    // arrows are lost when the player dies
    player_arrows = 3;
    if (player_arrows > MAX_ARROWS) player_arrows = MAX_ARROWS;

    // reset xp on death? keep as-is (do not change); leave player_xp unchanged
}

void player_set_pos(unsigned char x, unsigned char y) {
    p_x = x;
    p_y = y;
}

unsigned char player_get_x(void) {
    return p_x;
}

unsigned char player_get_y(void) {
    return p_y;
}

unsigned char player_get_lives(void) {
    return (player_stats & PLAYER_LIVES_MASK) >> PLAYER_LIVES_SHIFT;
}

unsigned char player_get_health(void) {
    return player_stats & PLAYER_HEALTH_MASK;
}

unsigned char player_get_level(void) {
    return player_level;
}

unsigned char player_get_arrows(void) {
    return player_arrows;
}

unsigned player_has_axe(void) {
    return (player_inventory & PLAYER_INV_AXE) != 0;
}

unsigned player_is_quiver_full(void) {
    return player_arrows >= MAX_ARROWS;
}

xp_t player_get_xp(void) {
    return player_xp;
}

/* Level thresholds: index = level, value = cumulative XP required to reach that level.
   thresholds[1] = 50 means reaching level 1 requires 50 XP (we start at level 1).
   Provide a table for several levels; beyond the table no further automatic leveling.
*/
static const xp_t level_thresholds[] = {
    0,        /* 0 - unused */
    50,       /* level 1 */
    125,      /* level 2 */
    250,      /* level 3 */
    750,      /* level 4 */
    2000,     /* level 5 */
    5000,     /* level 6 */
    12000,    /* level 7 */
    30000,    /* level 8 */
    75000,    /* level 9 */
    200000,   /* level 10 */
    500000,   /* level 11 */
    1200000,  /* level 12 */
    3000000,  /* level 13 */
    7500000,  /* level 14 */
    20000000, /* level 15 */
};

static const int MAX_TABLE_LEVEL = (sizeof(level_thresholds)/sizeof(level_thresholds[0]) - 1);

static xp_t get_threshold_for_level(int lvl) {
    if (lvl <= 0) return 0;
    if (lvl <= MAX_TABLE_LEVEL) return level_thresholds[lvl];
    return (xp_t)UINT32_MAX; /* no defined threshold beyond table */
}

void player_add_xp(xp_t amount) {
    if (amount == 0) return;
    /* add XP */
    player_xp += amount;

    /* level up while XP meets next-level thresholds */
    while (player_level < MAX_LEVEL) {
        int next_lvl = player_level + 1;
        xp_t next_thresh = get_threshold_for_level(next_lvl);
        if (next_thresh == (xp_t)UINT32_MAX) break; /* no further leveling defined */
        if (player_xp >= next_thresh) {
            /* level up: set new level, play jingle and flash border (no stat changes) */
            player_level = (unsigned char)next_lvl;
            ui_trigger_border(BORDER_DURATION, COLOR_LT_BLUE);
            play_levelup();
        } else {
            break;
        }
    }
}

void player_add_arrows(unsigned char count) {
    if (count == 0) return;
    unsigned int new_count = player_arrows + (unsigned int)count;
    if (new_count > MAX_ARROWS) new_count = MAX_ARROWS;
    player_arrows = (unsigned char)new_count;
}

unsigned player_has_key(void) {
    return (player_inventory & PLAYER_INV_KEY) != 0;
}

unsigned player_has_boat(void) {
    return (player_inventory & PLAYER_INV_BOAT) != 0;
}

unsigned player_has_crown_1(void) {
    return (player_inventory & PLAYER_INV_CROWN1) != 0;
}

unsigned player_has_crown_2(void) {
    return (player_inventory & PLAYER_INV_CROWN2) != 0;
}
