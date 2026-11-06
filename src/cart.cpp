#include "cart.h"
#include "cart_type.h"
#include "memory_bus.h"
#include "rtc.h"
#include <SDL3/SDL.h>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>

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


// Loads a Game Boy ROM from the specified filename into the cartridge_data buffer. Returns true if successful, false otherwise.
bool cart_load(const char* filename) {
	std::streampos size;
	std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
	if (file.is_open()) {
		size = file.tellg(); // Get the size of the file
		file.seekg(0, std::ios::beg); // Move to the beginning of the file
		file.read((char*)cartridge_data, MAX_CART_SIZE); // Read the file into the cartridge_data buffer
		file.close();
		cart_type_info cart_info = cart_type_data[cartridge_header->cartridge_type];
		if (size > MAX_CART_SIZE) 
		{
			printf("Warning: ROM size exceeds maximum cartridge size. Only the first %d bytes would be loaded.\n", MAX_CART_SIZE);
			return false;
		}
		if (cart_info.type == CART_TYPE::MBC1)
		{
			if (cart_info.has_battery && cart_info.has_ram)
			{
				cart_supports_saves = true;
			}
		}
		if (cart_info.type == CART_TYPE::MBC3)
		{
			if (!(cartridge_header->ram_size >= 0x00 && cartridge_header->ram_size <= 0x03))
			{
				printf("ROM %s is using MBC3 with invalid ERAM size value (0x%02X). This is not supported. Please check the ROM.\n", filename, cartridge_header->ram_size);
				return false;
			}
			if (!(cartridge_header->rom_size >= 0x00 && cartridge_header->rom_size <= 0x06))
			{
				printf("ROM %s is using MBC3 with invalid ROM size value (0x%02X). This is not supported. Please check the ROM.\n", filename, cartridge_header->rom_size);
				return false;
			}
			if (cart_info.has_battery && (cart_info.has_ram || cart_info.has_timer))
			{
				cart_supports_saves = true;
			}
			if (cart_info.has_timer) // All carts with timers have batteries.
			{
				rtc_init();
			}
		}

		if (cart_supports_saves)
		{
			cart_check_save(filename); // Check if a save file exists for this ROM and load it into ERAM if it does
		}

		strncpy(rom_filename_buffer, filename, CART_MAX_PATH - 1); // Store the ROM filename
		rom_filename_buffer[CART_MAX_PATH - 1] = '\0'; // strncpy doesn't terminate when the source fills the buffer
		cartridge_loaded = true;

		printf("ROM %s loaded, size: %lli bytes\n", filename, std::streamoff(size));
		return true;
	}
	printf("Failed to load ROM %s\n", filename);
	return false;
}

bool cart_check_save(const char* rom_filename) {
	if (cart_supports_saves) // Cart needs to have a battery and RAM or RTC to have a save file
	{
		if (cartridge_header->ram_size >= sizeof(cart_ram_size) / sizeof(cart_ram_size[0])) {
			printf("ROM %s has an invalid RAM size (0x%02X). This is not supported. Please check the ROM.\n", rom_filename, cartridge_header->ram_size);
			return false;
		}

		strncpy(save_filename_buffer, rom_filename, CART_MAX_PATH); // Copy the ROM filename to the save_filename buffer
		// Replace the extension with .sav
		char* dot = strrchr(save_filename_buffer, '.');
		if (dot) {
			strcpy(dot, ".sav");
		}
		else {
			strncat(save_filename_buffer, ".sav", CART_MAX_PATH - strlen(save_filename_buffer) - 1);
		}

		if (cart_ram_size[cartridge_header->ram_size].bank_count > 0) // Check if the RAM size is greater than 0
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
	std::streampos size;
	std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
	if (file.is_open()) {
		size = file.tellg(); // Get the size of the file
		file.seekg(0, std::ios::beg); // Move to the beginning of the file
		file.read((char*)save_data, ERAM_SIZE); // Read the file into the save_data buffer
		file.close();
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
		printf("Save file %s loaded, size: %lli bytes\n", filename, std::streamoff(size));
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
	// Check for existing save, if so, rename to .bak
	std::ifstream existing_save(filename);
	if (existing_save)
	{
		std::string backup_filename = std::string(filename) + ".bak";
		existing_save.close();
		// Check for existing backup and remove it if it exists
		std::ifstream existing_backup(backup_filename);
		if (existing_backup)
		{
			existing_backup.close();
			if (std::remove(backup_filename.c_str()) != 0)
			{
				printf("Warning: Failed to remove existing backup save file %s.\n", backup_filename.c_str());
			}
			else
			{
				printf("Existing backup save file %s removed.\n", backup_filename.c_str());
			}
		}
		if (std::rename(filename, backup_filename.c_str()) != 0)
		{
			printf("Warning: Failed to create backup of existing save file %s.\n", filename);
		} 
		else 
		{
			printf("Existing save file %s renamed to %s.\n", filename, backup_filename.c_str());
		}
	}
	std::ofstream file(filename, std::ios::out | std::ios::binary);
	if (file.is_open()) {
		file.write((const char*)eram, save_size);
		if (cart_has_rtc)
		{
			uint64_t utc_timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count(); // Get the current UTC timestamp
			rtc_create_save_bytes(utc_timestamp); // Create the RTC save bytes
			file.write((const char*)rtc_save_bytes, sizeof(rtc_save_bytes)); // Write the RTC save bytes to the file
		}
		file.close();
		std::ifstream verify_file(filename, std::ios::in | std::ios::binary);

		if (cart_has_rtc)
		{
			if (verify_file.seekg(0, std::ios::end).tellg() != (save_size + 48))
			{
				printf("Warning: Save file %s size does not match expected size (%u bytes + 48 bytes for RTC).\n", filename, save_size);
				return false;
			}
		}
		else
		{
			if (verify_file.seekg(0, std::ios::end).tellg() != save_size) {
				printf("Warning: Save file %s size does not match expected size (%u bytes).\n", filename, save_size);
				return false;
			}
		}
		
		verify_file.close();
		printf("Save file %s saved, size: %u bytes\n", filename, save_size);
		return true;
	} else {
		printf("Failed to open save file %s for writing.\n", filename);
		return false;
	}
}