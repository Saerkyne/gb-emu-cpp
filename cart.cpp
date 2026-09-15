#include "cart.h"
#include <iostream>
#include <fstream>
#include <Windows.h>

char runtime_path_buffer[MAX_PATH]; // Buffer to hold the runtime path
uint8_t cartridge_data[MAX_CART_SIZE]; // Will contain game rom
bool cartridge_loaded = false; // Flag to indicate if a cartridge is loaded
cart_header_struct* cartridge_header = (cart_header_struct*)(cartridge_data + 0x100); // Pointer to the cartridge header structure, which is located at offset 0x100 in the cartridge data


// Helper function to get path of currently running emulator
bool get_runtime_path()
{
	if (GetCurrentDirectory(MAX_PATH, runtime_path_buffer) == 0)
	{
		return false;
	}
	return true;
}

// Opens a default file dialog to select a Game Boy ROM. Not used currently, but could be useful for future GUI implementation.
bool cart_open_file()
{
	if (!get_runtime_path())
	{
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

	if (GetOpenFileNameA(&ofn))
	{
		cart_load(filename);
		return true;
	}
	return false;
}

void cart_print_info()
{
	printf("Entry point: %.2X%.2X%.2X%.2X\n", 
		cartridge_header->entry_point[0], 
		cartridge_header->entry_point[1], 
		cartridge_header->entry_point[2], 
		cartridge_header->entry_point[3]);
	printf("Title: %s\n", cartridge_header->title);
	printf("CGB Flag: %.2X\n", cartridge_header->cgb_flag);
	printf("New Licensee Code: %.2X%.2X\n", cartridge_header->new_licensee_code[0], cartridge_header->new_licensee_code[1]);
	printf("SGB Flag: %.2X\n", cartridge_header->sgb_flag);
	printf("Cartridge Type: %.2X\n", cartridge_header->cartridge_type);
	printf("ROM Size: %.2X\n", cartridge_header->rom_size);
	printf("RAM Size: %.2X\n", cartridge_header->ram_size);
	printf("Destination Code: %.2X\n", cartridge_header->destination_code);
	printf("Old Licensee Code: %.2X\n", cartridge_header->old_licensee_code);
	printf("Version: %.2X\n", cartridge_header->mask_rom_version_number);
	printf("Header Checksum: %.2X\n", cartridge_header->header_checksum);
	printf("Global Checksum: %.2X%.2X\n", cartridge_header->global_checksum_high, cartridge_header->global_checksum_low);
}


// Loads a Game Boy ROM from the specified filename into the cartridge_data buffer. Returns true if successful, false otherwise.
bool cart_load(const char* filename)
{
	std::streampos size;
	std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
	if (file.is_open())
	{
		size = file.tellg(); // Get the size of the file
		file.seekg(0, std::ios::beg); // Move to the beginning of the file
		file.read((char*)cartridge_data, MAX_CART_SIZE); // Read the file into the cartridge_data buffer
		file.close();
		printf("RM %s loaded, size: %lli bytes\n", filename, std::streamoff(size));
		return true;
	}
	printf("Failed to load ROM %s\n", filename);
	return false;
}