#include "cart.h"
#include "memory_bus.h"
#include "ppu.h"
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

lcd_registers* ppu_lcd_registers = nullptr;

void timer_on_div_write(const uint8_t) {}
void interrupt_flag_write(uint8_t) {}
void interrupt_enable_write(uint8_t) {}
void debug_log(const char*, ...) {}
uint8_t joypad_read_input() { return 0xFF; }
void joypad_write_input(uint8_t) {}
void ppu_on_lcd_control_write(const uint8_t) {}
void ppu_on_lcd_status_write(const uint8_t) {}
void ppu_on_lyc_write(const uint8_t) {}
void ppu_on_palette_write(const uint16_t) {}

void require(bool condition)
{
	if (!condition)
	{
		std::abort();
	}
}

void setup_cartridge(uint8_t type, uint8_t rom_size, uint8_t ram_size, uint32_t bank_count)
{
	std::memset(cartridge_data, 0, sizeof(cartridge_data));
	std::memset(eram, 0, ERAM_SIZE);
	cartridge_size = bank_count * 0x4000;
	for (uint32_t bank = 0; bank < bank_count; bank++)
	{
		std::memset(cartridge_data + (bank * 0x4000), static_cast<int>(bank), 0x4000);
	}
	cartridge_header->cartridge_type = type;
	cartridge_header->rom_size = rom_size;
	cartridge_header->ram_size = ram_size;
	memory_bus_reset_cartridge_state();
}

void test_rom_banks()
{
	setup_cartridge(0x11, 1, 0, 4);
	require(memory_bus_read(0x0000) == 0);
	require(memory_bus_read(0x4000) == 1);
	memory_bus_write(0x2000, 2);
	require(memory_bus_read(0x4000) == 2);
	memory_bus_write(0x2000, 0);
	require(memory_bus_read(0x4000) == 1);

	setup_cartridge(0x00, 0, 0, 2);
	require(memory_bus_read(0x4000) == 1);

	setup_cartridge(0x01, 1, 0, 4);
	memory_bus_write(0x2000, 2);
	require(memory_bus_read(0x4000) == 2);
}

void test_ram_banks_and_enable()
{
	setup_cartridge(0x12, 1, 3, 4);
	require(memory_bus_read(0xA000) == 0xFF);
	memory_bus_write(0x0000, 0x0A);
	memory_bus_write(0x4000, 1);
	memory_bus_write(0xA000, 0x31);
	memory_bus_write(0x4000, 3);
	memory_bus_write(0xA000, 0x73);
	require(memory_bus_read(0xA000) == 0x73);
	memory_bus_write(0x4000, 1);
	require(memory_bus_read(0xA000) == 0x31);
	memory_bus_write(0x4000, 4);
	require(memory_bus_read(0xA000) == 0xFF);
	memory_bus_write(0x0000, 0);
	require(memory_bus_read(0xA000) == 0xFF);
}

void test_rtc_registers_and_latching()
{
	setup_cartridge(0x10, 1, 3, 4);
	memory_bus_write(0x0000, 0x0A);
	memory_bus_write(0x4000, 0x08);
	memory_bus_write(0xA000, 0x12);
	memory_bus_write(0x6000, 0);
	memory_bus_write(0x6000, 1);
	memory_bus_write(0xA000, 0x34);
	require(memory_bus_read(0xA000) == 0x12);
	memory_bus_write(0x6000, 0);
	memory_bus_write(0x6000, 1);
	require(memory_bus_read(0xA000) == 0x34);

	setup_cartridge(0x12, 1, 3, 4);
	memory_bus_write(0x0000, 0x0A);
	memory_bus_write(0x4000, 0x08);
	require(memory_bus_read(0xA000) == 0xFF);
}

void test_battery_save_round_trip()
{
	const std::string rom_path = "mbc3_battery_test.gb";
	const std::string save_path = rom_path + ".sav";
	std::vector<uint8_t> rom(0x8000, 0);
	rom[0x147] = 0x10;
	rom[0x148] = 0;
	rom[0x149] = 3;
	{
		std::ofstream file(rom_path, std::ios::binary | std::ios::trunc);
		file.write(reinterpret_cast<const char*>(rom.data()), static_cast<std::streamsize>(rom.size()));
		require(static_cast<bool>(file));
	}

	require(cart_load(rom_path.c_str()));
	memory_bus_write(0x0000, 0x0A);
	memory_bus_write(0x4000, 0);
	memory_bus_write(0xA000, 0x5A);
	memory_bus_write(0x4000, 0x08);
	memory_bus_write(0xA000, 0x2A);
	cart_save_battery_data();

	cartridge_loaded = false;
	require(cart_load(rom_path.c_str()));
	memory_bus_write(0x0000, 0x0A);
	memory_bus_write(0x4000, 0);
	require(memory_bus_read(0xA000) == 0x5A);
	memory_bus_write(0x4000, 0x08);
	require(memory_bus_read(0xA000) == 0x2A);

	std::remove(save_path.c_str());
	std::remove(rom_path.c_str());
}

int main()
{
	test_rom_banks();
	test_ram_banks_and_enable();
	test_rtc_registers_and_latching();
	test_battery_save_round_trip();
	return 0;
}
