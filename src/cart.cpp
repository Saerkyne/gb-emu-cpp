#include "cart.h"
#include "cart_type.h"
#include "memory_bus.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <Windows.h>

char runtime_path_buffer[MAX_PATH]; // Buffer to hold the runtime path
uint8_t cartridge_data[MAX_CART_SIZE]; // Will contain game rom
bool cartridge_loaded = false; // Flag to indicate if a cartridge is loaded
uint32_t cartridge_size = 0;
cart_header_struct* cartridge_header = (cart_header_struct*)(cartridge_data + 0x100); // Pointer to the cartridge header structure, which is located at offset 0x100 in the cartridge data
std::string cartridge_save_path;


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
		cart_load(filename);
		return true;
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
uint32_t cartridge_ram_save_size()
{
	switch (cartridge_header->ram_size)
	{
	case 0x01: return 0x0800;
	case 0x02: return 0x2000;
	case 0x03: return 0x8000;
	case 0x04: return ERAM_SIZE;
	case 0x05: return ERAM_SIZE;
	default: return 0;
	}
}

void cart_save_battery_data()
{
	if (!cartridge_loaded || cartridge_save_path.empty())
	{
		return;
	}

	const cart_type_info& cart_info = cart_type_data[cartridge_header->cartridge_type];
	if (cart_info.type != CART_TYPE::MBC3 || !cart_info.has_battery)
	{
		return;
	}

	const uint32_t ram_size = cart_info.has_ram ? cartridge_ram_save_size() : 0;
	const uint8_t rtc_size = cart_info.has_timer ? 5 : 0;
	const uint8_t save_header[8] = {
		'G', 'B', 'S', 'V', 1,
		cartridge_header->cartridge_type,
		cartridge_header->ram_size,
		rtc_size
	};
	const std::string temporary_path = cartridge_save_path + ".tmp";
	std::ofstream file(temporary_path, std::ios::binary | std::ios::trunc);
	if (!file.is_open())
	{
		return;
	}

	file.write(reinterpret_cast<const char*>(save_header), sizeof(save_header));
	if (ram_size > 0)
	{
		file.write(reinterpret_cast<const char*>(eram), ram_size);
	}
	if (rtc_size > 0)
	{
		uint8_t rtc_registers[5] = {};
		memory_bus_get_rtc_registers(rtc_registers);
		file.write(reinterpret_cast<const char*>(rtc_registers), sizeof(rtc_registers));
	}
	file.close();
	if (!file)
	{
		std::remove(temporary_path.c_str());
		return;
	}

	if (!MoveFileExA(temporary_path.c_str(), cartridge_save_path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		std::remove(temporary_path.c_str());
	}
}

void cart_load_battery_data()
{
	std::memset(eram, 0, ERAM_SIZE);
	const cart_type_info& cart_info = cart_type_data[cartridge_header->cartridge_type];
	if (cart_info.type != CART_TYPE::MBC3 || !cart_info.has_battery)
	{
		return;
	}

	const uint32_t ram_size = cart_info.has_ram ? cartridge_ram_save_size() : 0;
	const uint8_t rtc_size = cart_info.has_timer ? 5 : 0;
	const uint32_t expected_size = 8 + ram_size + rtc_size;
	std::ifstream file(cartridge_save_path, std::ios::binary | std::ios::ate);
	if (!file.is_open() || file.tellg() != static_cast<std::streamoff>(expected_size))
	{
		return;
	}

	file.seekg(0, std::ios::beg);
	uint8_t save_header[8] = {};
	file.read(reinterpret_cast<char*>(save_header), sizeof(save_header));
	if (!file || save_header[0] != 'G' || save_header[1] != 'B' ||
		save_header[2] != 'S' || save_header[3] != 'V' || save_header[4] != 1 ||
		save_header[5] != cartridge_header->cartridge_type ||
		save_header[6] != cartridge_header->ram_size || save_header[7] != rtc_size)
	{
		return;
	}

	if (ram_size > 0)
	{
		file.read(reinterpret_cast<char*>(eram), ram_size);
	}
	if (rtc_size > 0)
	{
		uint8_t rtc_registers[5] = {};
		file.read(reinterpret_cast<char*>(rtc_registers), sizeof(rtc_registers));
		if (file)
		{
			memory_bus_set_rtc_registers(rtc_registers);
		}
	}
}

bool cart_load(const char* filename) {
	/* Previous loader implementation retained from this file:
	std::streampos size;
	std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
	if (file.is_open()) {
		size = file.tellg();
		file.seekg(0, std::ios::beg);
		file.read((char*)cartridge_data, MAX_CART_SIZE);
		file.close();
		return true;
	}
	return false;
	*/
	std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		printf("Failed to load ROM %s\n", filename);
		return false;
	}

	const std::streamoff size = file.tellg();
	if (size <= 0 || size > MAX_CART_SIZE) {
		printf("Invalid ROM size for %s\n", filename);
		return false;
	}

	std::vector<uint8_t> rom_data(static_cast<size_t>(size));
	file.seekg(0, std::ios::beg);
	file.read(reinterpret_cast<char*>(rom_data.data()), static_cast<std::streamsize>(rom_data.size()));
	if (!file) {
		printf("Failed to read ROM %s\n", filename);
		return false;
	}
	file.close();

	if (cartridge_loaded) {
		cart_save_battery_data();
	}

	std::memset(cartridge_data, 0, sizeof(cartridge_data));
	std::memcpy(cartridge_data, rom_data.data(), rom_data.size());
	cartridge_size = static_cast<uint32_t>(rom_data.size());
	cartridge_save_path = std::string(filename) + ".sav";
	cartridge_loaded = true;
	memory_bus_reset_cartridge_state();
	cart_load_battery_data();

	printf("RM %s loaded, size: %lli bytes\n", filename, static_cast<long long>(size));
	return true;
}