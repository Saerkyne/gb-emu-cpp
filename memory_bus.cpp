#include "memory_bus.h"
#include "cart.h"
#include "timer.h"
#include "interrupts.h"
#include <stdio.h>

uint8_t memory[MEMORY_SIZE];
uint8_t eram[ERAM_SIZE];

uint8_t memory_bus_read(const uint16_t addr) {
	if (addr >= 0x0000 && addr <= 0x3FFF) { // Read from ROM bank 00
		return cartridge_data[addr];
	}

	if (addr >= 0x4000 && addr <= 0x7FFF) { // Read from ROM bank 01~NN
		return cartridge_data[addr]; // For no MBC cartridges
	}

	if (addr >= 0x8000 && addr <= 0x9FFF) { // VRAM
		return memory[addr];
	}

	if (addr >= 0xA000 && addr <= 0xBFFF) { // External RAM
		const uint16_t eram_address = (addr - 0xA000);
		return eram[eram_address];
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

	if (addr >= 0x8000 && addr <= 0x9FFF) { // VRAM
		// TODO: If PPU is in mode 3 the cpu cannot access VRAM
		memory[addr] = value;
	}

	if (addr >= 0xA000 && addr <= 0xBFFF) { // ERAM
		eram[addr - 0xA000] = value;
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

	if (addr >= 0xFF00 && addr <= 0xFF7F) { // I/O registers
		if (addr == 0xFF02 && value == 0x81) { // Blargg tests serial output
			char c = memory[0xFF01];
			printf("%c\n", c);
		} else if (addr == 0xFF04) { // Timer DIV
			timer_on_div_write(value);
		} else if (addr == 0xFF07) { // Timer control
			memory[addr] = value & 0b00000111;
		} else if (addr == 0xFF0F) { // Interrupt flag write
			interrupt_flag_write(value);
		} else if (addr == 0xFF46) { // OAM DMA
			uint16_t source_addr = (value * 0x100);
			const uint16_t dest = 0xFE00;
			for (uint8_t i = 0; i < 160; i++) {
				memory[dest + i] = memory_bus_read(source_addr + i);
			}
		} else {
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