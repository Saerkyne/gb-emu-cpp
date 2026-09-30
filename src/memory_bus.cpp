#include "memory_bus.h"
#include "cart.h"
#include "cart_type.h"
#include "debug_log.h"
#include "interrupts.h"
#include "timer.h"
#include <stdio.h>

uint8_t memory[MEMORY_SIZE];
uint8_t eram[ERAM_SIZE];

uint16_t rom_bank_number = 0;
uint16_t ram_bank_number = 0;
bool ram_enabled = false;
bool rom_ram_mode_select = false;

uint8_t memory_bus_read(const uint16_t addr) {
	if (addr >= 0x0000 && addr <= 0x3FFF) { // Read from ROM bank 00
		return cartridge_data[addr];
	}

	const cart_type_info& cart_info = cart_type_data[cartridge_header->cartridge_type];

	if (addr >= 0x4000 && addr <= 0x7FFF) { // Read from ROM bank 01~NN
		if (cart_info.type == CART_TYPE::NO_MBC) {
			return cartridge_data[addr]; // For no MBC cartridges, return the data directly from the cartridge data array.
		} 
		else if (cart_info.type == CART_TYPE::MBC1) {
			// For MBC1 cartridges, calculate the offset based on the selected ROM bank number.
			const uint16_t ROM_BANK_SIZE = 0x4000; // 16k per ROM bank

			// ROM bank number is 1-based, so if the selected bank is 0, we should use bank 1 instead.
			uint8_t rom_bank = rom_bank_number > 0 ? rom_bank_number : 1;

			// Calculate the offset into the cartridge data array based on the selected ROM bank.
			const uint32_t offset = ROM_BANK_SIZE * (rom_bank - 1);

			// Return the data from the cartridge data array at the calculated offset plus the address within the bank.
			return cartridge_data[offset + addr];

		} 
		else if (cart_info.type == CART_TYPE::MBC2) {
			const uint16_t ROM_BANK_SIZE = 0x4000; // 16k per ROM bank, up to 16 banks

			uint8_t rom_bank = (rom_bank_number & 0x0F) > 0 ? (rom_bank_number & 0x0F) : 1;

			// Preserved fallback behavior while MBC2 mapping is incomplete.
			(void)ROM_BANK_SIZE;
			(void)rom_bank;
			return cartridge_data[addr];
		}

		return cartridge_data[addr];
	}


	if (addr >= 0x8000 && addr <= 0x9FFF) { // VRAM
		return memory[addr];
	}

	if (addr >= 0xA000 && addr <= 0xBFFF) { // External RAM
		if (cart_info.type == CART_TYPE::NO_MBC) {
			const uint16_t eram_address = (addr - 0xA000);
			return eram[eram_address];
		} 
		else if (cart_info.type == CART_TYPE::MBC1) {
			const uint16_t bank_offset = ram_bank_number * 0x2000; // 8k per RAM bank
			const uint16_t eram_address = (addr - 0xA000) + bank_offset;
			return eram[eram_address];
		}
	}

	if (addr >= 0xC000 && addr <= 0xCFFF) { // Work RAM 1
		return memory[addr];
	}

	if (addr >= 0xD000 && addr <= 0xDFFF) { // Work RAM 2
		return memory[addr];
	}

	if (addr >= 0xE000 && addr <= 0xFDFF) { // Echo RAM (Mirror of C000-DDFF)
		return memory[addr - 0x2000];
	}

	if (addr >= 0xFE00 && addr <= 0xFE9F) { // Object Attribute Memory (sprite attribute table)
		// TODO: If PPU mode == 2 return 0xFF
		return memory[addr];
	}

	if (addr >= 0xFEA0 && addr <= 0xFEFF) { // Unused memory area
		// TODO: return 0xFF if PPU is in mode 3
		return 0x00;
	}

	if (addr >= 0xFF00 && addr <= 0xFF7F) { // I/O Registers
		return memory[addr];
	}

	if (addr >= 0xFF80 && addr <= 0xFFFE) { // High RAM
		return memory[addr];
	}

	if (addr == 0xFFFF) { // Interrupt Enable Register
		return memory[addr];
	}

	return 0xFF;
}

void memory_bus_write(const uint16_t addr, const uint8_t value) {

	const cart_type_info& cart_info = cart_type_data[cartridge_header->cartridge_type];
	if (cart_info.type == CART_TYPE::MBC1) {
		if (addr >= 0x0000 && addr <= 0x1FFF) { // RAM Enable
			// When 0x0A (0b1010) is written to this register, enable RAM chip access.
			// When anything else is written, disable RAM chip access.
			// The register only checks the lower 4 bits of the value written, so 0x0A, 0x1A, 0x2A, etc. will all enable RAM access.
			ram_enabled = (value & 0x0F) == 0x0A;
		}

		if (addr >= 0x2000 && addr <= 0x3FFF) { // ROM Bank Register 1 (5 bits)
			// This value can never be 0b00000, it will always default to  0b00001
			// This means that half of the bank registers are not accessible.
			rom_bank_number = value & 0b00011111;

			// Compute ROM size in KB from cartridge header value, shifting left by size code. 
			// 32KB shifted left by 0 is 32KB, shifted left by 1 is 64KB, etc.
			const uint32_t rom_size = 32 * (1 << cartridge_header->rom_size);

			// Calculate the number of ROM banks based on the ROM size. Each bank is 16KB,
			// so divide the total ROM size by 16KB to get the number of banks.
			const uint8_t number_of_rom_banks = rom_size / 16;

			// Clamp to valid existing bank index
			rom_bank_number = rom_bank_number % number_of_rom_banks;
		}

		if (addr >= 0x4000 && addr <= 0x5FFF) { // ROM Bank Register 2 (2 bits) or RAM Bank Register (2 bits)
			if (rom_ram_mode_select) {
				// if RAM mode is selected, this register selects the RAM bank number (0-3)
				ram_bank_number = value & 0b00000011;
			} else {
				// If ROM mode is selected, this register selects the upper 2 bits of the ROM bank number (5 bits total)
				rom_bank_number = (rom_bank_number & 0b00011111) | ((value & 0b00000011) << 5);
			}
		}

		if (addr >= 0x6000 && addr <= 0x7FFF) { // ROM/RAM Mode Select (1 bit)
			// if false, ROM banking mode is selected (up to 512KB ROM, 8KB RAM)
			// if true, RAM banking mode is selected (up to 128KB ROM, 32KB RAM)
			rom_ram_mode_select = value & 1; // This is a 1-bit register
		}
	}

	if (cart_info.type == CART_TYPE::MBC2) {
		// MBC2 has a built-in 512x4 bits (256 bytes) of RAM
		// RAM and ROM banks alternate in the same 0x0000-0x3FFF space, with the A8 address signal choosing the register.
		// If the A8 signal is 0b0, then RAM is selected, and if the A8 signal is 0b1, then ROM is selected.
		if (addr >= 0x0000 && addr <= 0x3FFF) {
			if ((addr & 0x0100) == 0) {				  // A8 is 0, RAM is selected
				ram_enabled = (value & 0x0F) == 0x0A; // Enable/disable RAM
			} 
			else {								  // A8 is 1, ROM is selected
				rom_bank_number = value & 0x0F;		  // Select ROM bank (4 bits)
				if (rom_bank_number == 0) {
					rom_bank_number = 1; // Bank 0 cannot be selected, default to bank 1
				}
			}
		}

		if (addr >= 0xA000 && addr <= 0xBFFF) { // MBC2 RAM area (512 x 4 bits)
			// Only 0xA000 - 0xA1FF are used, higher values are wrapped around. 0xA200 accesses 0xA000, etc.
			eram[(addr - 0xA000) & 0x01FF] = (value & 0x0F); // Store only the lower nibble (4 bits)
		}
	}

	if (addr >= 0x8000 && addr <= 0x9FFF) { // VRAM
		// TODO: If PPU is in mode 3 the cpu cannot access VRAM
		memory[addr] = value;
	}

	if (addr >= 0xA000 && addr <= 0xBFFF) { // ERAM
		if (cart_info.type == CART_TYPE::NO_MBC) {
			// If the cartridge has no MBC, the external RAM is directly mapped to the address range 0xA000-0xBFFF.
			eram[addr - 0xA000] = value;
		} 
		else if (cart_info.type == CART_TYPE::MBC1) {
			// If the cartridge has an MBC1, the external RAM is banked and the selected RAM bank is determined by the ram_bank_number variable.
			const uint16_t RAM_BANK_SIZE = 0x2000; // 8k per ROM bank
			const uint32_t offset = RAM_BANK_SIZE * ram_bank_number;
			const uint16_t eram_address = offset + (addr - 0xA000);
			eram[eram_address] = value;
		}
	}

	if (addr >= 0xC000 && addr <= 0xCFFF) { // WRAM 1
		memory[addr] = value;
	}

	if (addr >= 0xD000 && addr <= 0xDFFF) { // WRAM 2
		memory[addr] = value;
	}

	if (addr >= 0xE000 && addr <= 0xFDFF) { // Echo RAM
		memory[addr - 0x2000] = value;
	}

	if (addr >= 0xFE00 && addr <= 0xFE9F) { // OAM Data
		// TODO: If PPU is in mode 2 or 3 the cpu cannot access OAM
		memory[addr] = value;
	}

	if (addr >= 0xFEA0 && addr <= 0xFEFF) { // Unused memory area
											// All writes here are ignored
	}

	if (addr >= 0xFF00 && addr <= 0xFF7F) {	   // I/O registers
		if (addr == 0xFF02 && value == 0x81) { // Blargg tests serial output

			char c = memory[0xFF01];
			debug_log("%c", c);

		} 
		else if (addr == 0xFF04) { // Timer DIV
			timer_on_div_write(value);
		} 
		else if (addr == 0xFF07) { // Timer control
			memory[addr] = value & 0b00000111;
		} 
		else if (addr == 0xFF0F) { // Interrupt flag write
			interrupt_flag_write(value);
		} 
		else if (addr == 0xFF46) { // OAM DMA
			uint16_t source_addr = (value * 0x100);
			const uint16_t dest = 0xFE00;
			for (uint8_t i = 0; i < 160; i++) {
				memory[dest + i] = memory_bus_read(source_addr + i);
			}
		} 
		else {
			memory[addr] = value;
		}
	}

	if (addr >= 0xFF80 && addr <= 0xFFFE) { // HRAM
		memory[addr] = value;
	}

	if (addr == 0xFFFF) { // Interrupt Enable
		interrupt_enable_write(value);
	}
}