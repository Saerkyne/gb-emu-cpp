# Example: Address Dispatch with Cartridge-Specific Helper Functions

A clean way to structure `memory_bus_write` is to dispatch by address range first, then call small helpers for the cartridge-specific ranges.

## Why this shape

- Keeps the main function readable.
- Makes cartridge logic local to only the ranges that need it.
- Avoids repeating `cart_info.type` checks across unrelated address blocks.

## Example structure

```cpp
static void write_rom_area(uint16_t addr, uint8_t value, const cart_type_info& cart_info) {
	if (cart_info.type == CART_TYPE::MBC1) {
		if (addr <= 0x1FFF) {
			ram_enabled = (value & 0x0F) == 0x0A;
			return;
		}

		if (addr <= 0x3FFF) {
			rom_bank_number = value & 0b00011111;

			const uint32_t rom_size = 32 * (1 << cartridge_header->rom_size);
			const uint8_t number_of_rom_banks = rom_size / 16;
			rom_bank_number = rom_bank_number % number_of_rom_banks;
			return;
		}

		if (addr <= 0x5FFF) {
			if (rom_ram_mode_select) {
				ram_bank_number = value & 0b00000011;
			} else {
				rom_bank_number = (rom_bank_number & 0b00011111) | ((value & 0b00000011) << 5);
			}
			return;
		}

		if (addr <= 0x7FFF) {
			rom_ram_mode_select = value & 1;
			return;
		}
	}

	if (cart_info.type == CART_TYPE::MBC2) {
		if ((addr & 0x0100) == 0) {
			ram_enabled = (value & 0x0F) == 0x0A;
		} else {
			rom_bank_number = value & 0x0F;
			if (rom_bank_number == 0) {
				rom_bank_number = 1;
			}
		}
	}
}

static void write_eram_area(uint16_t addr, uint8_t value, const cart_type_info& cart_info) {
	if (cart_info.type == CART_TYPE::NO_MBC) {
		eram[addr - 0xA000] = value;
		return;
	}

	if (cart_info.type == CART_TYPE::MBC1) {
		const uint16_t bank_offset = ram_bank_number * 0x2000;
		eram[(addr - 0xA000) + bank_offset] = value;
		return;
	}

	if (cart_info.type == CART_TYPE::MBC2) {
		eram[(addr - 0xA000) & 0x01FF] = value & 0x0F;
	}
}

void memory_bus_write(uint16_t addr, uint8_t value) {
	const cart_type_info& cart_info = cart_type_data[cartridge_header->cartridge_type];

	if (addr <= 0x7FFF) {
		write_rom_area(addr, value, cart_info);
		return;
	}

	if (addr >= 0x8000 && addr <= 0x9FFF) {
		memory[addr] = value;
		return;
	}

	if (addr >= 0xA000 && addr <= 0xBFFF) {
		write_eram_area(addr, value, cart_info);
		return;
	}

	if (addr >= 0xC000 && addr <= 0xCFFF) {
		memory[addr] = value;
		return;
	}

	if (addr >= 0xD000 && addr <= 0xDFFF) {
		memory[addr] = value;
		return;
	}

	if (addr >= 0xE000 && addr <= 0xFDFF) {
		memory[addr - 0x2000] = value;
		return;
	}

	if (addr >= 0xFE00 && addr <= 0xFE9F) {
		memory[addr] = value;
		return;
	}

	if (addr >= 0xFEA0 && addr <= 0xFEFF) {
		return;
	}

	if (addr >= 0xFF00 && addr <= 0xFF7F) {
		if (addr == 0xFF02 && value == 0x81) {
			debug_log("%c", memory[0xFF01]);
		} else if (addr == 0xFF04) {
			timer_on_div_write(value);
		} else if (addr == 0xFF07) {
			memory[addr] = value & 0b00000111;
		} else if (addr == 0xFF0F) {
			interrupt_flag_write(value);
		} else if (addr == 0xFF46) {
			uint16_t source_addr = value * 0x100;
			const uint16_t dest = 0xFE00;
			for (uint8_t i = 0; i < 160; i++) {
				memory[dest + i] = memory_bus_read(source_addr + i);
			}
		} else {
			memory[addr] = value;
		}
		return;
	}

	if (addr >= 0xFF80 && addr <= 0xFFFE) {
		memory[addr] = value;
		return;
	}

	if (addr == 0xFFFF) {
		interrupt_enable_write(value);
	}
}
```

## Suggested rule of thumb

- Use **address dispatch** at the top level.
- Use **cartridge-specific helpers** only inside ranges that vary by mapper.
- Keep each helper focused on one address region.

If wanted, the same pattern can be expanded into separate helpers for:

- ROM control
- ERAM access
- I/O registers
- DMA handling
