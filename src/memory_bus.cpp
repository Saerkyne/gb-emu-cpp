#include "memory_bus.h"
#include "cart.h"
#include "cart_type.h"
#include "rtc.h"
#include "timer.h"
#include "interrupts.h"
#include "debug_log.h"
#include "ppu.h"
#include "joypad.h"
#include <stdio.h>

uint8_t memory[MEMORY_SIZE];
uint8_t eram[ERAM_SIZE];

uint16_t rom_bank_number = 1; // Default to bank 1 for MBC1 and MBC3
uint16_t mbc1_bank2 = 0; // Used for MBC1 when in ROM banking mode
uint16_t ram_bank_number = 0; // Used as a selector not just bank number in MBC3
bool ram_enabled = false;
bool rom_ram_mode_select = false;

uint8_t memory_bus_read(const uint16_t addr) 
{

	const cart_type_info& cart_info = cart_type_data[cartridge_header->cartridge_type];

	if (addr >= 0x0000 && addr <= 0x3FFF) // Read from ROM bank 00
	{
		if (cart_info.type == CART_TYPE::NO_MBC)
		{
			return cartridge_data[addr]; // For no MBC cartridges
		}
		else if (cart_info.type == CART_TYPE::MBC1)
		{
			if (rom_ram_mode_select)
			{
				uint8_t rom_bank = (mbc1_bank2 << 5) & ((1 << (cartridge_header->rom_size + 1)) - 1); // Mask the rom bank number based on the ROM size
				return  cartridge_data[rom_bank * 0x4000 + (addr - 0x0000)]; // Calculate the address based on the selected ROM bank
			}
			return cartridge_data[addr]; // For MBC1, bank 00 is always accessible
		}
		else if (cart_info.type == CART_TYPE::MBC3)
		{
			return cartridge_data[addr]; // For MBC3, bank 00 is always accessible
		}
		return cartridge_data[addr];
	}

	

	if (addr >= 0x4000 && addr <= 0x7FFF) // Read from ROM bank 01~NN
	{ 
		if (cart_info.type == CART_TYPE::NO_MBC) 
		{
			return cartridge_data[addr]; // For no MBC cartridges
		} 
		else if (cart_info.type == CART_TYPE::MBC1) 
		{
			const uint16_t ROM_BANK_SIZE = 0x4000; // 16k per ROM bank
			uint8_t rom_bank = ((mbc1_bank2 << 5) | rom_bank_number) & ((1 << (cartridge_header->rom_size + 1)) - 1); // Combine the lower 5 bits with the upper 2 bits to form a 7-bit bank number
			const uint32_t bank_offset = rom_bank * ROM_BANK_SIZE;
			const uint32_t rom_address = bank_offset + (addr - 0x4000);
			return cartridge_data[rom_address];
		}
		else if (cart_info.type == CART_TYPE::MBC3)
		{
			uint8_t rom_bank = rom_bank_number;
			if (rom_bank == 0x00)
			{
				rom_bank = 0x01; // Bank 0 is not accessible, default to bank 1
			}
			const uint16_t ROM_BANK_SIZE = 0x4000; // 16k per ROM bank
			rom_bank = rom_bank & ((1 << (cartridge_header->rom_size + 1)) - 1); // Mask the rom bank number based on the ROM size
			const uint32_t bank_offset = rom_bank * ROM_BANK_SIZE;
			const uint32_t rom_address = bank_offset + (addr - 0x4000);
			return cartridge_data[rom_address];
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
			if (!ram_enabled)
			{
				return 0xFF; // RAM is disabled, return 0xFF
			}
			if (rom_ram_mode_select) // RAM banking mode
			{
				const uint16_t RAM_BANK_SIZE = 0x2000; // 8k per RAM bank
				const uint16_t ram_bank = mbc1_bank2 & (cart_ram_size[cartridge_header->ram_size].bank_count - 1); // Mask the ram_bank_number based on the number of banks available
				const uint32_t bank_offset = ram_bank * RAM_BANK_SIZE;
				const uint32_t eram_address = (addr - 0xA000) + bank_offset;
				return eram[eram_address];
			}
			else
			{
				const uint32_t eram_address = (addr - 0xA000);
				return eram[eram_address];
			}
		}
		else if (cart_info.type == CART_TYPE::MBC3)
		{
			if (!ram_enabled)
			{
				return 0xFF; // RAM/RTC is disabled, return 0xFF
			}
			if (ram_bank_number >= 0x08 && ram_bank_number <= 0x0C) // RTC register selector
			{
				if (!cart_info.has_timer)
				{
					return 0xFF; // no timer, return 0xFF
				}
				else
				{
					return rtc_register_read(ram_bank_number);
				}
			}
			if (cartridge_header->ram_size < sizeof(cart_ram_size) / sizeof(cart_ram_size[0])) // Check that the cartridge RAM size is within the bounds of the cart_ram_size array
			{
				if (cartridge_header->ram_size == 0x00)
				{
					return 0xFF; // No RAM available, return 0xFF
				}
				else if (cartridge_header->ram_size == 0x01)
				{
					return 0xFF; // Unused RAM size code, return 0xFF
				}
				else if (ram_bank_number >= 0x00 && ram_bank_number <= 0x07) // Functions as a selector, not a RAM bank value
				{
					const uint16_t RAM_BANK_SIZE = 0x2000; // 8k per RAM bank
					uint8_t masked_ram_bank_number = ram_bank_number & (cart_ram_size[cartridge_header->ram_size].bank_count - 1); // Mask the ram_bank_number based on the number of banks available
					const uint32_t bank_offset = masked_ram_bank_number * RAM_BANK_SIZE;
					const uint32_t eram_address = (addr - 0xA000) + bank_offset;
					return eram[eram_address];
				}
			}
			return 0xFF; // Default case
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
			
			if (rom_bank_number == 0)
			{
				rom_bank_number = 1; // Bank 0 is not accessible, default to bank 1
			}
		}

		if (addr >= 0x4000 && addr <= 0x5FFF) // ROM Bank Register 2 (2 bits) or RAM Bank Register (2 bits)
		{
			mbc1_bank2 = value & 0b00000011; // This is a 2-bit register
		}

		if (addr >= 0x6000 && addr <= 0x7FFF) 
		{
			rom_ram_mode_select = value & 1; // This is a 1-bit register
		}
	}

	if (cart_info.type == CART_TYPE::MBC3)
	{
		if (addr >= 0x0000 && addr <= 0x1FFF) // RAM and Timer Enable
		{
			ram_enabled = (value & 0x0F) == 0x0A;
		}

		if (addr >= 0x2000 && addr <= 0x3FFF) // ROM Bank Number (7 bits)
		{
			rom_bank_number = value & 0b01111111;
			if (rom_bank_number == 0)
			{
				rom_bank_number = 1; // Bank 0 is not accessible, default to bank 1
			}
		}

		if (addr >= 0x4000 && addr <= 0x5FFF) // RAM Bank Number (2 bits) or RTC Register Select (4 bits)
		{
			ram_bank_number = value;
		}

		if (addr >= 0x6000 && addr <= 0x7FFF) // Latch Clock Data
		{
			rtc_latch_write(value);
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
			if (!ram_enabled)
			{
				return; // RAM is disabled, ignore the write
			}

			if (rom_ram_mode_select)
			{
				const uint16_t RAM_BANK_SIZE = 0x2000; // 8k per RAM bank
				const uint16_t ram_bank = mbc1_bank2 & (cart_ram_size[cartridge_header->ram_size].bank_count - 1); // Mask the ram_bank_number based on the number of banks available
				const uint32_t offset = RAM_BANK_SIZE * ram_bank;
				const uint32_t eram_address = offset + (addr - 0xA000);
				eram[eram_address] = value;
			}
			else
			{
				
				eram[addr - 0xA000] = value;
			}
			
		}
		else if (cart_info.type == CART_TYPE::MBC3)
		{
			if (!ram_enabled)
			{
				 // RAM/RTC is disabled, ignore the write
				 return;
			}
			if (ram_bank_number >= 0x08 && ram_bank_number <= 0x0C) // RTC register selector
			{
				if (!cart_info.has_timer)
				{
					return; // no timer, ignore the write
				}
				else
				{
					rtc_register_write(ram_bank_number, value);
				}
			}
			else if (ram_bank_number >= 0x00 && ram_bank_number <= 0x07) // Functions as a selector, not a RAM bank value
			{
				if (cartridge_header->ram_size < sizeof(cart_ram_size) / sizeof(cart_ram_size[0])) // Check that the cartridge RAM size is within the bounds of the cart_ram_size array
				{
					if (cartridge_header->ram_size == 0x00)
					{
						// No RAM available, ignore the write
					}
					else if (cartridge_header->ram_size == 0x01)
					{
						// Unused RAM size code, ignore the write
					}
					else
					{
						const uint16_t RAM_BANK_SIZE = 0x2000; // 8k per RAM bank
						uint8_t masked_ram_bank_number = ram_bank_number & (cart_ram_size[cartridge_header->ram_size].bank_count - 1); // Mask the ram_bank_number based on the number of banks available
						const uint32_t bank_offset = masked_ram_bank_number * RAM_BANK_SIZE;
						const uint32_t eram_address = (addr - 0xA000) + bank_offset;
						eram[eram_address] = value;
					}
				}
			}
			return; // Exit after handling MBC3 RAM/RTC write
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