#ifndef JTTP_GLOBALS_H
#define JTTP_GLOBALS_H

#include <stdint.h>

// --- OSCAR64 MEMORY CONFIGURATION ---
#pragma section( charset, 0 )
#pragma section( metadata, 0 )
#pragma section( registry, 0, , , bss ) // bss type means it's not in the PRG file size

// REGION 1: Code and data before the graphics ($0A00 - $2000)
#pragma region( lower_reg, 0x0a00, 0x2000, , , {code, data} )

// REGION 2: The Charset ($2000 - $2800)
#pragma region( charset_reg, 0x2000, 0x2800, , , {charset} )

// REGION 3: Metadata ($2800 - $3000)
#pragma region( metadata_reg, 0x2800, 0x3000, , , {metadata} )

// REGION 4: Everything else ($3000 - $A000)
#pragma region( main, 0x3000, 0xa000, , , {code, data, bss, heap, stack} )

// REGION 5: The Registry ($C000 - $D000)
#pragma region( registry_reg, 0xc000, 0xd000, , , {registry} )

unsigned char get_game_mode(void);
uint8_t get_current_dungeon(void);
void switch_to_overworld(void) ;
void switch_to_fog_of_war(uint8_t d_idx);

#define MODE_MAP 0x01
#define MODE_FOG 0x02

#endif  // JTTP_GLOBALS_H
