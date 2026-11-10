#include "cart.h"
#include "cart_type.h"
#include "memory_bus.h"
#include "rtc.h"
#include <SDL3/SDL.h>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>

char runtime_path_buffer[CART_MAX_PATH]; // Buffer to hold the runtime path
char rom_filename_buffer[CART_MAX_PATH]; // Buffer to hold the ROM filename
char save_filename_buffer[CART_MAX_PATH]; // Buffer to hold the save filename
bool cart_supports_saves = false; // Flag to indicate if the currently loaded cartridge supports saves
uint32_t save_size; // Size of the save file, calculated based on the cartridge RAM size
uint8_t save_data[ERAM_SIZE]; // Buffer to hold the save data
uint8_t cartridge_data[MAX_CART_SIZE]; // Will contain game rom
bool cartridge_loaded = false; // Flag to indicate if a cartridge is loaded
cart_header_struct* cartridge_header = (cart_header_struct*)(cartridge_data + 0x100); // Pointer to the cartridge header structure, which is located at offset 0x100 in the cartridge data


// Helper function to get path of currently running emulator
bool get_runtime_path() {
	const char* base_path = SDL_GetBasePath();
	if (base_path == NULL) {
		return false;
	}
	strncpy(runtime_path_buffer, base_path, CART_MAX_PATH);
	return true;
}

void cart_unload() {
	if (cartridge_loaded && cart_supports_saves)
	{
		cart_save_to_file(save_filename_buffer); // Uses save_size and save_filename_buffer, so it runs before they are cleared
	}

	cartridge_loaded = false;
	cart_supports_saves = false;
	save_size = 0;
	rom_filename_buffer[0] = '\0';
	save_filename_buffer[0] = '\0';

	// The next cartridge starts from the same blank slate as a fresh start of the emulator
	memset(cartridge_data, 0, MAX_CART_SIZE);
	memset(save_data, 0, ERAM_SIZE);
	memset(eram, 0, ERAM_SIZE); // Cartridge RAM belongs to the cartridge being removed
}

void cart_print_info() {
	printf("Entry point: %.2X%.2X%.2X%.2X\n", 
		cartridge_header->entry_point[0], 
		cartridge_header->entry_point[1], 
		cartridge_header->entry_point[2], 
		cartridge_header->entry_point[3]);
	printf("Title: %s\n", cartridge_header->title);
	printf("CGB Flag: %.2X\n", cartridge_header->cgb_flag);
	printf("New Licensee Code: %.2X%.2X\n", cartridge_header->new_licensee_code[0], cartridge_header->new_licensee_code[1]);
	printf("SGB Flag: %.2X\n", cartridge_header->sgb_flag);
	printf("Cartridge Type: %.2X (%s)\n", cartridge_header->cartridge_type, cart_type_data[cartridge_header->cartridge_type].readable_name);
	printf("ROM Size: %.2X\n", cartridge_header->rom_size);
	printf("RAM Size: %.2X\n", cartridge_header->ram_size);
	printf("Destination Code: %.2X\n", cartridge_header->destination_code);
	printf("Old Licensee Code: %.2X\n", cartridge_header->old_licensee_code);
	printf("Version: %.2X\n", cartridge_header->mask_rom_version_number);
	printf("Header Checksum: %.2X\n", cartridge_header->header_checksum);
	printf("Global Checksum: %.2X%.2X\n", cartridge_header->global_checksum_high, cartridge_header->global_checksum_low);
}

static bool cart_header_is_valid(const uint8_t* rom, const char* filename) {
	const cart_header_struct* header = (const cart_header_struct*)(rom + 0x100);

	uint8_t checksum = 0;
	for (uint16_t address = 0x0134; address <= 0x014C; address++)
	{
		checksum = checksum -rom[address] - 1;
	}
	if (checksum != header->header_checksum)
	{
		printf("ROM %s has an invalid header checksum (0x%02X). Expected 0x%02X. This is not supported. Please check the ROM.\n", filename, header->header_checksum, checksum);
		return false;
	}

	if (cart_type_data[header->cartridge_type].type == CART_TYPE::MBC3)
	{
		if (header->ram_size > 0x03)
		{
			printf("ROM %s is using MBC3 with invalid ERAM size value (0x%02X). This is not supported. Please check the ROM.\n", filename, header->ram_size);
			return false;
		}
		if (header->rom_size > 0x06)
		{
			printf("ROM %s is using MBC3 with invalid ROM size value (0x%02X). This is not supported. Please check the ROM.\n", filename, header->rom_size);
			return false;
		}
	}
	return true;
}

bool cart_check_rom_file(const char* filename) {
	SDL_IOStream* file = SDL_IOFromFile(filename, "rb");
	if (file == NULL) 
	{
		printf("Failed to open ROM %s: %s\n", filename, SDL_GetError());
		return false;
	}

	uint8_t header_bytes[0x150]; // Everything from the start of the file through the end of the header
	const Sint64 size = SDL_GetIOSize(file);
	const size_t bytes_read = (size >= (Sint64)sizeof(header_bytes)) ? SDL_ReadIO(file, header_bytes, sizeof(header_bytes)) : 0;
	SDL_CloseIO(file);

	if (size < (Sint64)sizeof(header_bytes) || bytes_read != sizeof(header_bytes)) {
		printf("ROM %s is too small to contain a valid header. Size: %lld bytes\n", filename, (long long)size);
		return false;
	}
	if (size > MAX_CART_SIZE) {
		printf("ROM %s is too large to be loaded. Size: %lld bytes, Max: %d bytes\n", filename, (long long)size, MAX_CART_SIZE);
		return false;
	}
	return cart_header_is_valid(header_bytes, filename);
}

// Loads a Game Boy ROM from the specified filename into the cartridge_data buffer. Returns true if successful, false otherwise.
bool cart_load(const char* filename) {
	SDL_IOStream* file = SDL_IOFromFile(filename, "rb"); // SDL takes UTF-8 paths on every platform, including Windows
	if (file == NULL) 
	{
		printf("Failed to load ROM %s: %s\n", filename, SDL_GetError());
		return false;
	}

	const Sint64 size = SDL_GetIOSize(file); // Size of the file in bytes, or -1 if it cannot be determined
	if (size < 0)
	{
		printf("Failed to read the size of ROM %s: %s\n", filename, SDL_GetError());
		SDL_CloseIO(file);
		return false;
	}

	if (size > MAX_CART_SIZE)
	{
		// Checked before reading, so an oversized file never touches cartridge_data
		printf("Warning: ROM size exceeds the maximum cartridge size of %d bytes. The ROM was not loaded.\n", MAX_CART_SIZE);
		SDL_CloseIO(file);
		return false;
	}

	const size_t bytes_read = SDL_ReadIO(file, cartridge_data, (size_t)size); // Read the file into the cartridge_data buffer
	SDL_CloseIO(file); // Close the file

	if (bytes_read != (size_t)size) {
		printf("Failed to read ROM %s: %s\n", filename, SDL_GetError());
		return false;
	}

	if (size < 0x150 || !cart_header_is_valid(cartridge_data, filename))
	{
		return false; // cart_header_is_valid printed the reason
	}

	cart_type_info cart_info = cart_type_data[cartridge_header->cartridge_type];
	switch (cart_info.type)
	{
	case CART_TYPE::NO_MBC:
		if (cart_info.has_battery && cart_info.has_ram)
		{
			cart_supports_saves = true;
		}
		break;
	case CART_TYPE::MBC1:
		if (cart_info.has_battery && cart_info.has_ram)
		{
			cart_supports_saves = true;
		}
		break;
	case CART_TYPE::MBC2:
		if (cart_info.has_battery)
		{
			cart_supports_saves = true;
		}
		break;
	case CART_TYPE::MBC3:
		if (cart_info.type == CART_TYPE::MBC3)
		{
			if (cart_info.has_battery && (cart_info.has_ram || cart_info.has_timer))
			{
				cart_supports_saves = true;
			}
			if (cart_info.has_timer) // All carts with timers have batteries.
			{
				rtc_init();
			}
		}
		break;
	default:
		break;
	}

	if (cart_supports_saves)
	{
		cart_check_save(filename); // Check if a save file exists for this ROM and load it into ERAM if it does
	}

	strncpy(rom_filename_buffer, filename, CART_MAX_PATH - 1); // Store the ROM filename
	rom_filename_buffer[CART_MAX_PATH - 1] = '\0'; // strncpy doesn't terminate when the source fills the buffer
	cartridge_loaded = true;

	printf("ROM %s loaded, size: %lld bytes\n", filename, (long long)size);
	return true;
}

bool cart_check_save(const char* rom_filename) {
	if (cart_supports_saves) // Cart needs to have a battery and RAM or RTC to have a save file
	{
		if (cartridge_header->ram_size == 0x00 && cart_type_data[cartridge_header->cartridge_type].type == CART_TYPE::MBC2)
		{
			printf("ROM %s is using MBC2 with built-in RAM.\n", rom_filename);
		}
		else if (cartridge_header->ram_size >= sizeof(cart_ram_size) / sizeof(cart_ram_size[0])) {
			printf("ROM %s has an invalid RAM size (0x%02X). This is not supported. Please check the ROM.\n", rom_filename, cartridge_header->ram_size);
			return false;
		}

		// The save name is the ROM path with its extension replaced by .sav. Only a dot in the
		// file name itself counts, not one in a folder name like "my.games/tetris".
		const char* name_start = rom_filename;
		const char* last_slash = strrchr(rom_filename, '/');
		const char* last_backslash = strrchr(rom_filename, '\\'); // Windows accepts either separator
		if (last_slash && last_slash + 1 > name_start) {
			name_start = last_slash + 1;
		}
		if (last_backslash && last_backslash + 1 > name_start) {
			name_start = last_backslash + 1;
		}
		const char* dot = strrchr(name_start, '.');
		const size_t stem_length = dot ? (size_t)(dot - rom_filename) : strlen(rom_filename);

		// ".sav" can be longer than the extension it replaces, so check that the result fits
		if (stem_length + strlen(".sav") >= CART_MAX_PATH) {
			printf("The path to ROM %s is too long to put a save file next to it. Progress will not be saved.\n", rom_filename);
			cart_supports_saves = false; // No valid save name, so shutdown must not try to write one
			return false;
		}
		memcpy(save_filename_buffer, rom_filename, stem_length);
		strcpy(save_filename_buffer + stem_length, ".sav");

		if (cart_type_data[cartridge_header->cartridge_type].type == CART_TYPE::MBC2)
		{
			save_size = 512; // MBC2 has 512 x 4 bits of built-in RAM

			if (cart_open_save_file(save_filename_buffer)) // Check if a save exists. 
			{
				// logic to load save file into ERAM
				memcpy(eram, save_data, save_size); // Copy the save data into the ERAM buffer
				return true;
			}
			return false;
		}
		else if (cart_ram_size[cartridge_header->ram_size].bank_count > 0) // Check if the RAM size is greater than 0
		{
			save_size = cart_ram_size[cartridge_header->ram_size].bank_count * 8 * 1024; // Calculate the expected save file size based on the RAM size
			
			if (cart_open_save_file(save_filename_buffer)) // Check if a save exists. 
			{
				// logic to load save file into ERAM
				memcpy(eram, save_data, save_size); // Copy the save data into the ERAM buffer
				return true;
			}
			return false;
		}

		if ((cart_ram_size[cartridge_header->ram_size].bank_count == 0) && (cart_type_data[cartridge_header->cartridge_type].has_timer == true)) // Check if the RAM size is 0
		{
			save_size = 0;
			if (cart_open_save_file(save_filename_buffer)) // Check if a save exists. 
			{
				// logic to load save file into ERAM
				memcpy(eram, save_data, save_size); // Copy the save data into the ERAM buffer
				printf("ROM %s has a RAM size of 0 but does have an RTC, which means a save size of 44 or 48 bytes.\n", rom_filename);
				return true;
			}
			else
			{
				printf("No save found for ROM %s.\n", rom_filename);
				return false;
			}
		}
		printf("ROM %s has a bank count of 0 and no RTC, which means no save data.\n", rom_filename);
		return false;
	}
	else 
	{
		printf("ROM %s does not have a battery and RAM or RTC, so no save data is available.\n", rom_filename);
		return false;
	}
}

bool cart_open_save_file(const char* filename) {
	SDL_IOStream* file = SDL_IOFromFile(filename, "rb"); // SDL takes UTF-8 paths on every platform, including Windows
	if (file != NULL) {
		const Sint64 size = SDL_GetIOSize(file); // Size of the file in bytes, or -1 if it can't be determined
		const size_t bytes_expected = (size >= 0 && size < (Sint64)ERAM_SIZE) ? (size_t)size : ERAM_SIZE;
		const size_t bytes_read = SDL_ReadIO(file, save_data, bytes_expected); // Read the file into the save_data buffer
		SDL_CloseIO(file);
		if (size < 0 || bytes_read != bytes_expected) {
			printf("Failed to read save file %s: %s\n", filename, SDL_GetError());
			return false;
		}
		uint8_t footer_length = 0; // Initialize footer length to 0
		if (size < save_size)
		{
			printf("Warning: Save file size is smaller than expected save size (%u bytes).\n", save_size);
			return false;
		}
		else if (size == (save_size + 44) && cart_has_rtc)
		{
			printf("Save has a 44 byte footer.\n");
			footer_length = 44;
		}
		else if (size == (save_size + 48) && cart_has_rtc)
		{
			printf("Save has a 48 byte footer.\n");
			footer_length = 48;
		}
		else if (size > save_size && !cart_has_rtc)
		{
			printf("Warning: Save file size is larger than expected save size (%u bytes) and has no RTC.\n", save_size);
			return false;
		}

		if (footer_length != 0)
		{
			memcpy(rtc_save_bytes, save_data + save_size, footer_length); // Copy the footer data into the rtc_save_bytes buffer
			
			uint64_t current_utc_timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count(); // Get the current UTC timestamp
			uint64_t loaded_utc_timestamp = rtc_load_save_bytes(current_utc_timestamp, footer_length); // Load the RTC registers from the save file footer; save the loaded timestamp

			if (current_utc_timestamp > loaded_utc_timestamp)
			{
				uint64_t seconds_to_catch_up = current_utc_timestamp - loaded_utc_timestamp;
				rtc_catch_up(seconds_to_catch_up); // Catch up the RTC registers by the difference in seconds
			}
		}
		printf("Save file %s loaded, size: %lld bytes\n", filename, (long long)size);
		return true;
	}
	else
	{
		printf("No save found.\n");
		return false;
	}
}

bool cart_save_to_file(const char* filename) {

	if (!cartridge_loaded) { // Nothing was ever loaded, so there is nothing to save
		return false;
	}

	if (!cart_supports_saves) {
		printf("Cartridge does not support saves.\n");
		return false;
	}
		// Keep the previous save as a .bak. SDL_RenamePath replaces an existing backup on
	// every platform, so the old backup doesn't need to be removed first.
	if (SDL_GetPathInfo(filename, NULL)) // True when something already exists at this path
	{
		const std::string backup_filename = std::string(filename) + ".bak";
		if (SDL_RenamePath(filename, backup_filename.c_str()))
		{
			printf("Existing save file %s renamed to %s.\n", filename, backup_filename.c_str());
		}
		else
		{
			printf("Warning: Failed to create backup of existing save file %s: %s\n", filename, SDL_GetError());
		}
	}

	SDL_IOStream* file = SDL_IOFromFile(filename, "wb");
	if (file == NULL) {
		printf("Failed to open save file %s for writing: %s\n", filename, SDL_GetError());
		return false;
	}

	uint32_t expected_size = save_size;
	bool write_ok = SDL_WriteIO(file, eram, save_size) == save_size;
	if (cart_has_rtc)
	{
		uint64_t utc_timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count(); // Get the current UTC timestamp
		rtc_create_save_bytes(utc_timestamp); // Create the RTC save bytes
		write_ok = (SDL_WriteIO(file, rtc_save_bytes, sizeof(rtc_save_bytes)) == sizeof(rtc_save_bytes)) && write_ok; // Write the RTC save bytes to the file
		expected_size += sizeof(rtc_save_bytes);
	}
	const bool closed_ok = SDL_CloseIO(file); // Closing flushes the data, so it can fail too

	// Confirm what actually landed on disk
	SDL_PathInfo info;
	if (!write_ok || !closed_ok || !SDL_GetPathInfo(filename, &info) || info.size != expected_size)
	{
		printf("Warning: Save file %s was not written correctly (expected %u bytes): %s\n", filename, expected_size, SDL_GetError());
		return false;
	}

	printf("Save file %s saved, size: %u bytes\n", filename, expected_size);
	return true;
}