#include "cart.h"
#include "cart_type.h"
#include "memory_bus.h"
#include <iostream>
#include <fstream>
#include <Windows.h>

char runtime_path_buffer[MAX_PATH]; // Buffer to hold the runtime path
char rom_filename_buffer[MAX_PATH]; // Buffer to hold the ROM filename
char save_filename_buffer[MAX_PATH]; // Buffer to hold the save filename
bool cart_supports_saves = false; // Flag to indicate if the currently loaded cartridge supports saves
uint32_t save_size; // Size of the save file, calculated based on the cartridge RAM size
uint8_t save_data[ERAM_SIZE]; // Buffer to hold the save data
uint8_t cartridge_data[MAX_CART_SIZE]; // Will contain game rom
bool cartridge_loaded = false; // Flag to indicate if a cartridge is loaded
cart_header_struct* cartridge_header = (cart_header_struct*)(cartridge_data + 0x100); // Pointer to the cartridge header structure, which is located at offset 0x100 in the cartridge data


// Helper function to get path of currently running emulator
bool get_runtime_path() {
	if (GetCurrentDirectoryA(MAX_PATH, runtime_path_buffer) == 0) {
		return false;
	}
	return true;
}

bool cart_open_file() {
	if (!get_runtime_path()) {
		return false;
	}
	
	char filename[MAX_PATH];

	OPENFILENAMEA ofn;       // common dialog box structure
	ZeroMemory(&filename, sizeof(filename)); // Initialize filename buffer
	ZeroMemory(&ofn, sizeof(ofn)); // Initialize OPENFILENAME
	ofn.lStructSize = sizeof(ofn); // Size of the structure
	ofn.hwndOwner = NULL; // If you have a window to center over, put its handle here
	ofn.lpstrFilter = "GB ROM Files\0*.gb;*.gbc\0All Files\0*.*\0"; // Filter for file types
	ofn.lpstrFile = filename; // Buffer to store the selected file name
	ofn.lpstrInitialDir = runtime_path_buffer; // Initial directory
	ofn.nMaxFile = MAX_PATH; // Maximum file name length
	ofn.lpstrTitle = "Select a Game Boy ROM"; // Dialog title
	ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST; // Flags for the dialog

	if (GetOpenFileNameA(&ofn)) {
		if (cart_load(filename)) {
			strncpy(rom_filename_buffer, filename, MAX_PATH); // Store the ROM filename
			return true;
		}
		return false;
	}
	return false;
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
	printf("Cartridge Type: %.2X\n", cartridge_header->cartridge_type, cart_type_data[cartridge_header->cartridge_type].readable_name);
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
		if (size > MAX_CART_SIZE) 
		{
			printf("Warning: ROM size exceeds maximum cartridge size. Only the first %d bytes would be loaded.\n", MAX_CART_SIZE);
			return false;
		}
		if (cart_type_data[cartridge_header->cartridge_type].type == CART_TYPE::MBC3)
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
			if (cart_type_data[cartridge_header->cartridge_type].has_battery && cart_type_data[cartridge_header->cartridge_type].has_ram)
			{
				cart_supports_saves = true;
			}
		}

		if (cart_supports_saves)
		{
			cart_check_save(filename); // Check if a save file exists for this ROM and load it into ERAM if it does
		}

		printf("ROM %s loaded, size: %lli bytes\n", filename, std::streamoff(size));
		return true;
	}
	printf("Failed to load ROM %s\n", filename);
	return false;
}

bool cart_check_save(const char* rom_filename) {
	if (cart_type_data[cartridge_header->cartridge_type].has_battery && cart_type_data[cartridge_header->cartridge_type].has_ram) // Cart needs to have a battery and RAM to have a save file
	{
		if (cartridge_header->ram_size >= sizeof(cart_ram_size) / sizeof(cart_ram_size[0])) {
			printf("ROM %s has an invalid RAM size (0x%02X). This is not supported. Please check the ROM.\n", rom_filename, cartridge_header->ram_size);
			return false;
		}
		if (cart_ram_size[cartridge_header->ram_size].bank_count > 0) // Check if the RAM size is greater than 0
		{
			save_size = cart_ram_size[cartridge_header->ram_size].bank_count * 8 * 1024; // Calculate the expected save file size based on the RAM size
			strncpy(save_filename_buffer, rom_filename, MAX_PATH); // Copy the ROM filename to the save_filename buffer
			// Replace the extension with .sav
			char* dot = strrchr(save_filename_buffer, '.');
			if (dot) {
				strcpy(dot, ".sav");
			} else {
				strncat(save_filename_buffer, ".sav", MAX_PATH - strlen(save_filename_buffer) - 1);
			}
			if (cart_open_save_file(save_filename_buffer)) // Check if a save exists. 
			{
				// logic to load save file into ERAM
				memcpy(eram, save_data, save_size); // Copy the save data into the ERAM buffer
				return true;
			}
			return false;
		}
		printf("ROM %s has a bank count of 0, which means no save data.\n", rom_filename);
		return false;
	}
	else 
	{
		printf("ROM %s does not have a battery and RAM, so no save data is available.\n", rom_filename_buffer);
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
		if (size > save_size)
		{
			printf("Warning: Save file size exceeds expected save size (%u bytes). Nothing will be loaded.\n", save_size);
			return false;
		}
		if (size < save_size)
		{
			printf("Warning: Save file size is smaller than expected save size (%u bytes).\n", save_size);
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
		file.close();
		std::ifstream verify_file(filename, std::ios::in | std::ios::binary);
		if (verify_file.seekg(0, std::ios::end).tellg() != save_size) {
			printf("Warning: Save file %s size does not match expected size (%u bytes).\n", filename, save_size);
			return false;
		}
		verify_file.close();
		printf("Save file %s saved, size: %u bytes\n", filename, save_size);
		return true;
	} else {
		printf("Failed to open save file %s for writing.\n", filename);
		return false;
	}
}