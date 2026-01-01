#ifndef ASSETS_H
#define ASSETS_H

#include <stdint.h>

// --- Asset Data (Linker-Placed via memory_map.h) ---
// Using these names ensures map.c and graphics.c point to 
// the stable regions defined in your memory map.
extern const uint8_t charset_data[2048];           // Located at $2000
extern const uint8_t chartileset_data[256];        // Located at $2800
extern const uint8_t chartileset_attrib_L1_data[64]; // Located at $2900
extern const uint8_t chartileset_tag_data[64];     // Located at $2940
extern const uint8_t map_data[220];                // Located at $2A00

// --- Global Settings ---
extern const uint8_t colr_vic_bg0;

#endif