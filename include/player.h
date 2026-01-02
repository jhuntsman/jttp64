// player.h -- player position API
#ifndef JTTP_PLAYER_H
#define JTTP_PLAYER_H

/* Packed stats bit layout (player_stats)
	bits 7-4 : lives (4 bits)
	bits 3-0 : health (4 bits)
*/
#define PLAYER_LIVES_SHIFT 4
#define PLAYER_LIVES_MASK  0xF0
#define PLAYER_HEALTH_MASK 0x0F

/* Inventory bitflags (player_inventory) */
#define PLAYER_INV_AXE     (1 << 0)
#define PLAYER_INV_KEY     (1 << 1)
#define PLAYER_INV_BOAT    (1 << 2)
#define PLAYER_INV_CROWN1  (1 << 3)
#define PLAYER_INV_CROWN2  (1 << 4)
#define PLAYER_INV_MASK_ALL (PLAYER_INV_AXE|PLAYER_INV_KEY|PLAYER_INV_BOAT|PLAYER_INV_CROWN1|PLAYER_INV_CROWN2)

/* Default starting lives */
#define MAX_LIVES 3

/* Quiver */
#define MAX_ARROWS 30

/* Maximum allowed player level */
#define MAX_LEVEL 99

/* Experience / Score type: allow scores into the millions (use 32-bit unsigned) */
#include <stdint.h>
typedef uint32_t xp_t;

/* Player helpers */
unsigned player_is_quiver_full(void);
xp_t player_get_xp(void);
void player_add_xp(xp_t amount);
void player_add_arrows(unsigned char count);


void init_player(void);
void reset_player(void);

void player_set_pos(unsigned char x, unsigned char y);
unsigned char player_get_x(void);
unsigned char player_get_y(void);
unsigned char player_get_lives(void);
unsigned char player_get_health(void);
unsigned char player_get_level(void);
unsigned char player_get_arrows(void);
unsigned player_has_axe(void);
unsigned player_has_key(void);
unsigned player_has_boat(void);
unsigned player_has_crown_1(void);
unsigned player_has_crown_2(void);

#endif // JTTP_PLAYER_H
