#ifndef ASSETS_H
#define ASSETS_H

#include <stdint.h>

// --- Global Project Settings ---
extern const uint8_t colr_vic_bg0; // Light Green

// --- CharSet Data (2048 bytes) ---
// Raw pixels for 256 characters (8 bytes each) [cite: 32, 33]
extern const uint8_t charset_data[2048];

// --- CharTileSet Data (256 bytes) ---
// Defines which 4 characters (2x2) make up each of the 64 Tile IDs [cite: 38, 39]
extern const uint8_t chartileset_data[256];

// --- Tile Colors (Attributes) ---
// One color byte per 2x2 tile [cite: 40, 41]
extern const uint8_t chartileset_attrib_L1_data[64];

// --- Tile Tags (For Collision/Logic) ---
// One tag byte per tile (set to $00 by default in your file) [cite: 42, 43]
extern const uint8_t chartileset_tag_data[64];

// --- Map Data (220 bytes) ---
// 20x11 Grid of Tile IDs 
extern const uint8_t map_data[220];

#endif