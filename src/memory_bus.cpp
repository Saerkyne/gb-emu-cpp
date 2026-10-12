#include "memory_bus.h"
#include "cart.h"
#include "cart_type.h"
#include "timer.h"
#include "interrupts.h"
#include "debug_log.h"
#include "ppu.h"
#include "joypad.h"
#include <stdio.h>

uint8_t memory[MEMORY_SIZE];
uint8_t eram[ERAM_SIZE];

uint16_t rom_bank_number = 0;
uint16_t ram_bank_number = 0;
bool ram_enabled = false;
bool rom_ram_mode_select = false;

struct mbc3_mapper_state {
	uint8_t rom_bank;
	uint8_t ram_rtc_select;
	bool ram_rtc_enabled;
	uint8_t latch_value;
	uint8_t rtc_registers[5];
	uint8_t latched_rtc_registers[5];
	bool rtc_latched;
};

mbc3_mapper_state mbc3_state = { 1, 0, false, 0, {}, {}, false };

uint32_t cartridge_rom_bank_count()
{
	uint32_t bank_count = 0;
	if (cartridge_header->rom_size <= 8)
	{
		bank_count = 2u << cartridge_header->rom_size;
	}
	else if (cartridge_header->rom_size == 0x52)
	{
		bank_count = 72;
	}
	else if (cartridge_header->rom_size == 0x53)
	{
		bank_count = 80;
	}
	else if (cartridge_header->rom_size == 0x54)
	{
		bank_count = 96;
	}

	const uint32_t maximum_bank_count = MAX_CART_SIZE / 0x4000;
	if (bank_count == 0 || bank_count > maximum_bank_count)
	{
		bank_count = maximum_bank_count;
	}
	const uint32_t loaded_bank_count = cartridge_size / 0x4000;
	if (loaded_bank_count > 0 && bank_count > loaded_bank_count)
	{
		bank_count = loaded_bank_count;
	}
	return bank_count;
}

uint32_t cartridge_ram_capacity()
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

void memory_bus_reset_cartridge_state()
{
	rom_bank_number = 0;
	ram_bank_number = 0;
	ram_enabled = false;
	rom_ram_mode_select = false;
	mbc3_state = { 1, 0, false, 0, {}, {}, false };
}

void memory_bus_get_rtc_registers(uint8_t registers[5])
{
	for (uint8_t i = 0; i < 5; i++)
	{
		registers[i] = mbc3_state.rtc_registers[i];
	}
}

void memory_bus_set_rtc_registers(const uint8_t registers[5])
{
	for (uint8_t i = 0; i < 5; i++)
	{
		mbc3_state.rtc_registers[i] = registers[i];
		mbc3_state.latched_rtc_registers[i] = registers[i];
	}
	mbc3_state.rtc_latched = false;
}

uint8_t memory_bus_read(const uint16_t addr) 
{
	if (addr >= 0x0000 && addr <= 0x3FFF) // Read from ROM bank 00
	{
		return cartridge_data[addr];
	}

	const cart_type_info& cart_info = cart_type_data[cartridge_header->cartridge_type];

	if (addr >= 0x4000 && addr <= 0x7FFF) // Read from ROM bank 01~NN
	{
		if (cart_info.type == CART_TYPE::NO_MBC)
		{
			return cartridge_data[addr]; // For no MBC cartridges
		}
		else if (cart_info.type == CART_TYPE::MBC1) // Banks $20/$40/$60 are not accessible in MBC1, so we need to wrap around
		{
			const uint16_t ROM_BANK_SIZE = 0x4000; // 16k per ROM bank
			uint8_t rom_bank = rom_bank_number > 0 ? rom_bank_number : 1;
			const uint32_t offset = ROM_BANK_SIZE * (rom_bank - 1);
			return cartridge_data[offset + addr];
		}
		else if (cart_info.type == CART_TYPE::MBC3) // Same as MBC1, but banks $20/$40/$60 are accessible in MBC3
		{
			/* Previous MBC3 branch shared MBC1 bank state:
			const uint16_t ROM_BANK_SIZE = 0x4000;
			uint8_t rom_bank = rom_bank_number > 0 ? rom_bank_number : 1;
			const uint32_t offset = ROM_BANK_SIZE * (rom_bank - 1);
			return cartridge_data[offset + addr];
			*/
			const uint32_t bank_count = cartridge_rom_bank_count();
			const uint32_t rom_bank = mbc3_state.rom_bank % bank_count;
			const uint32_t offset = (rom_bank * 0x4000) + (addr - 0x4000);
			return cartridge_data[offset];
		}
		return cartridge_data[addr];
	}

	if (addr >= 0x8000 && addr <= 0x9FFF) // VRAM
	{
		if (LCDS_PPU_MODE == LCDS_MODE_3_DRAWING)
		{
			return 0xFF;
		}
		return memory[addr];
	}

	if (addr >= 0xA000 && addr <= 0xBFFF) // External RAM
	{ 
		if (cart_info.type == CART_TYPE::NO_MBC) 
		{
			const uint16_t eram_address = (addr - 0xA000);
			return eram[eram_address];
		} 
		else if (cart_info.type == CART_TYPE::MBC1) 
		{
			const uint16_t bank_offset = ram_bank_number * 0x2000; // 8k per RAM bank
			const uint16_t eram_address = (addr - 0xA000) + bank_offset;
			return eram[eram_address];
		}
		else if (cart_info.type == CART_TYPE::MBC3)
		{
			if (!mbc3_state.ram_rtc_enabled)
			{
				return 0xFF;
			}

			if (mbc3_state.ram_rtc_select <= 0x03 && cart_info.has_ram)
			{
				const uint32_t ram_capacity = cartridge_ram_capacity();
				const uint32_t bank_count = ram_capacity / 0x2000 > 0 ? ram_capacity / 0x2000 : 1;
				if (ram_capacity == 0 || mbc3_state.ram_rtc_select >= bank_count)
				{
					return 0xFF;
				}

				const uint32_t ram_address = (mbc3_state.ram_rtc_select * 0x2000) + (addr - 0xA000);
				return eram[ram_address % ram_capacity];
			}

			if (mbc3_state.ram_rtc_select >= 0x08 && mbc3_state.ram_rtc_select <= 0x0C && cart_info.has_timer)
			{
				const uint8_t rtc_index = mbc3_state.ram_rtc_select - 0x08;
				return mbc3_state.rtc_latched
					? mbc3_state.latched_rtc_registers[rtc_index]
					: mbc3_state.rtc_registers[rtc_index];
			}

			return 0xFF;
		}
	}
	if (addr >= 0xC000 && addr <= 0xCFFF) // Work RAM 1
	{
		return memory[addr];
	}

	if (addr >= 0xD000 && addr <= 0xDFFF) // Work RAM 2
	{
		return memory[addr];
	}

	if (addr >= 0xE000 && addr <= 0xFDFF) // Echo RAM (Mirror of C000-DDFF)
	{
		return memory[addr - 0x2000];
	}

	if (addr >= 0xFE00 && addr <= 0xFE9F) // Object Attribute Memory (sprite attribute table)
	{
		if (LCDS_PPU_MODE == LCDS_MODE_2_OAM_SEARCH ||
			LCDS_PPU_MODE == LCDS_MODE_3_DRAWING)
		{
			return 0xFF;
		}
		return memory[addr];
	}

	if (addr >= 0xFEA0 && addr <= 0xFEFF) // Unused memory area
	{
		if (LCDS_PPU_MODE == LCDS_MODE_3_DRAWING)
		{
			return 0xFF;
		}
		return 0x00;
	}

	if (addr >= 0xFF00 && addr <= 0xFF7F) // I/O Registers
	{
		if (addr == 0xFF00) // Joypad input
		{
			return joypad_read_input();	
		}
		return memory[addr];
	}

	if (addr >= 0xFF80 && addr <= 0xFFFE) // High RAM
	{
		return memory[addr];
	}

	if (addr == 0xFFFF) // Interrupt Enable Register
	{
		return memory[addr];
	}

	return 0xFF;
}

void memory_bus_write(const uint16_t addr, const uint8_t value) 
{

	const cart_type_info& cart_info = cart_type_data[cartridge_header->cartridge_type];
	if (cart_info.type == CART_TYPE::MBC1) 
	{
		if (addr >= 0x0000 && addr <= 0x1FFF) // RAM Enable
		{
			// When 0x0A (0b1010) is written to this register, enable RAM chip access.
			// When anything else is written, disable RAM chip access.
			// The register only checks the lower 4 bits of the value written, so 0x0A, 0x1A, 0x2A, etc. will all enable RAM access.
			ram_enabled = (value & 0x0F) == 0x0A;
		}

		if (addr >= 0x2000 && addr <= 0x3FFF) // ROM Bank Register 1 (5 bits)
		{
			// This value can never be 0b00000, it will always default to  0b00001
			// This means that half of the bank registers are not accessible.
			rom_bank_number = value & 0b00011111;
			const uint32_t rom_size = 32 * (1 << cartridge_header->rom_size);
			const uint8_t number_of_rom_banks = rom_size / 16;
			rom_bank_number = rom_bank_number % number_of_rom_banks;
		}

		if (addr >= 0x4000 && addr <= 0x5FFF) // ROM Bank Register 2 (2 bits) or RAM Bank Register (2 bits)
		{
			if (rom_ram_mode_select) 
			{
				ram_bank_number = value & 0b00000011;
			} 
			else 
			{
				rom_bank_number = (rom_bank_number & 0b00011111) | ((value & 0b00000011) << 5);
			}
		}

		if (addr >= 0x6000 && addr <= 0x7FFF) 
		{
			rom_ram_mode_select = value & 1; // This is a 1-bit register
		}
	}
	else if (cart_info.type == CART_TYPE::MBC3)
	{
		if (addr >= 0x0000 && addr <= 0x1FFF)
		{
			mbc3_state.ram_rtc_enabled = (value & 0x0F) == 0x0A;
		}

		if (addr >= 0x2000 && addr <= 0x3FFF)
		{
			const uint32_t bank_count = cartridge_rom_bank_count();
			mbc3_state.rom_bank = value & 0x7F;
			mbc3_state.rom_bank %= bank_count;
			if (mbc3_state.rom_bank == 0)
			{
				mbc3_state.rom_bank = 1;
			}
		}

		if (addr >= 0x4000 && addr <= 0x5FFF)
		{
			mbc3_state.ram_rtc_select = value;
		}

		if (addr >= 0x6000 && addr <= 0x7FFF)
		{
			const uint8_t latch_value = value & 1;
			if (cart_info.has_timer && mbc3_state.latch_value == 0 && latch_value == 1)
			{
				for (uint8_t i = 0; i < 5; i++)
				{
					mbc3_state.latched_rtc_registers[i] = mbc3_state.rtc_registers[i];
				}
				mbc3_state.rtc_latched = true;
			}
			mbc3_state.latch_value = latch_value;
		}
	}

	if (addr >= 0x8000 && addr <= 0x9FFF)  // VRAM
	{
		if (LCDS_PPU_MODE == LCDS_MODE_3_DRAWING)
		{
			return;
		}
		memory[addr] = value;
	}

	if (addr >= 0xA000 && addr <= 0xBFFF) // ERAM
	{
		if (cart_info.type == CART_TYPE::NO_MBC) 
		{
			eram[addr - 0xA000] = value;
		} 
		else if (cart_info.type == CART_TYPE::MBC1) 
		{
			const uint16_t RAM_BANK_SIZE = 0x2000; // 8k per ROM bank
			const uint32_t offset = RAM_BANK_SIZE * ram_bank_number;
			const uint16_t eram_address = offset + (addr - 0xA000);
			eram[eram_address] = value;
		}
		else if (cart_info.type == CART_TYPE::MBC3)
		{
			if (!mbc3_state.ram_rtc_enabled)
			{
				return;
			}

			if (mbc3_state.ram_rtc_select <= 0x03 && cart_info.has_ram)
			{
				const uint32_t ram_capacity = cartridge_ram_capacity();
				const uint32_t bank_count = ram_capacity / 0x2000 > 0 ? ram_capacity / 0x2000 : 1;
				if (ram_capacity > 0 && mbc3_state.ram_rtc_select < bank_count)
				{
					const uint32_t ram_address = (mbc3_state.ram_rtc_select * 0x2000) + (addr - 0xA000);
					eram[ram_address % ram_capacity] = value;
				}
				return;
			}

			if (mbc3_state.ram_rtc_select >= 0x08 && mbc3_state.ram_rtc_select <= 0x0C && cart_info.has_timer)
			{
				const uint8_t rtc_index = mbc3_state.ram_rtc_select - 0x08;
				if (rtc_index == 0 || rtc_index == 1)
				{
					mbc3_state.rtc_registers[rtc_index] = value & 0x3F;
				}
				else if (rtc_index == 2)
				{
					mbc3_state.rtc_registers[rtc_index] = value & 0x1F;
				}
				else if (rtc_index == 4)
				{
					mbc3_state.rtc_registers[rtc_index] = value & 0xC1;
				}
				else
				{
					mbc3_state.rtc_registers[rtc_index] = value;
				}
				return;
			}
		}
	}

	if (addr >= 0xC000 && addr <= 0xCFFF) // WRAM 1
	{
		memory[addr] = value;
	}

	if (addr >= 0xD000 && addr <= 0xDFFF) // WRAM 2
	{
		memory[addr] = value;
	}

	if (addr >= 0xE000 && addr <= 0xFDFF) // Echo RAM
	{
		memory[addr - 0x2000] = value;
	}

	if (addr >= 0xFE00 && addr <= 0xFE9F) // OAM Data
	{
		if (LCDS_PPU_MODE == LCDS_MODE_2_OAM_SEARCH ||
			LCDS_PPU_MODE == LCDS_MODE_3_DRAWING)
		{
			return;
		}
		memory[addr] = value;
	}

	if (addr >= 0xFEA0 && addr <= 0xFEFF) // Unused memory area
	{
		// All writes here are ignored
	}

	if (addr >= 0xFF00 && addr <= 0xFF7F) // I/O registers
	{
		if (addr == 0xFF00) // Joypad input
		{
			joypad_write_input(value);
		}
		else if (addr == 0xFF02 && value == 0x81) // Blargg tests serial output
		{
			
			char c = memory[0xFF01];
			debug_log("%c", c);
			
		} 
		else if (addr == 0xFF04) // Timer DIV
		{
			timer_on_div_write(value);
		} 
		else if (addr == 0xFF07) // Timer control
		{
			memory[addr] = value & 0b00000111;
		} 
		else if (addr == 0xFF0F) // Interrupt flag write
		{
			interrupt_flag_write(value);
		}
		else if (addr == 0xFF40)
		{
			ppu_on_lcd_control_write(value);
		}
		else if (addr == 0xFF41)
		{
			ppu_on_lcd_status_write(value);
		}
		else if (addr == 0xFF45)
		{
			ppu_on_lyc_write(value);
		}
		else if (addr == 0xFF46) // OAM DMA
		{	
			uint16_t source_addr = (value * 0x100);
			const uint16_t dest = 0xFE00;
			for (uint8_t i = 0; i < 160; i++) {
				memory[dest + i] = memory_bus_read(source_addr + i);
			}
		}
		else if (addr >= 0xFF47 && addr <= 0xFF49)
		{
			memory[addr] = value;
			ppu_on_palette_write(addr);
		}
		else 
		{
			memory[addr] = value;
		}
	}

	if (addr >= 0xFF80 && addr <= 0xFFFE) // HRAM
	{
		memory[addr] = value;
	}

	if (addr == 0xFFFF) // Interrupt Enable
	{
		interrupt_enable_write(value);
	}

	
}