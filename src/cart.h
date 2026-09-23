#pragma once

#include <stdint.h>

const int MAX_CART_SIZE = 1024 * 1024;		  // 1 MB or 8Mbit
extern uint8_t cartridge_data[MAX_CART_SIZE]; // Will contain game rom
extern bool cartridge_loaded;				  // Flag to indicate if a cartridge is loaded

struct cart_header_struct {
	uint8_t entry_point[4];			 // Entry point for the cartridge
	uint8_t nintendo_logo[48];		 // Nintendo logo data
	uint8_t title[15];				 // Game title
	uint8_t cgb_flag;				 // CGB flag
	uint8_t new_licensee_code[2];	 // New licensee code, only matters if old_licensee_code is 0x33
	uint8_t sgb_flag;				 // SGB flag
	uint8_t cartridge_type;			 // Cartridge type, describes the memory bank controller and other features
	uint8_t rom_size;				 // ROM size
	uint8_t ram_size;				 // RAM size
	uint8_t destination_code;		 // Destination code, 0x00 for Japanese, 0x01 for non-Japanese
	uint8_t old_licensee_code;		 // Old licensee code, 0x33 means use new_licensee_code instead
	uint8_t mask_rom_version_number; // Specifies ROM version number, usually 0x00
	uint8_t header_checksum;		 // 8-bit checksum from bytes 0x0134-0x014C, used to verify the header
	uint8_t global_checksum_high;	 // High byte of the 16-bit (high-endian) global checksum, used to verify the entire ROM
	uint8_t global_checksum_low;	 // Low byte of the 16-bit (high-endian) global checksum, used to verify the entire ROM
};

extern cart_header_struct* cartridge_header; // Pointer to the cartridge header structure

bool cart_open_file();
void cart_print_info();
bool cart_load(const char* filename);