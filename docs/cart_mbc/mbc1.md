# MBC1

[Back to roadmap](../memory_bus_cart_mbc_roadmap.md)

## Header types

| Header byte | `cart_type.h` description |
| --- | --- |
| `01` | MBC1 |
| `02` | MBC1 + RAM |
| `03` | MBC1 + RAM + battery |

Battery save persistence is deferred. MBC1 control-register decoding and RAM gating are still required for all three variants.

## Hardware model

MBC1 presents two CPU ROM windows of 16 KiB and an optional 8 KiB external-RAM window. Its registers are changed by 
writes to the ROM address space.

```text
CPU control-write range     Register                     Bits used
0000–1FFF                   RAM enable                   value[3:0] == A
2000–3FFF                   ROM bank low                 value[4:0]
4000–5FFF                   ROM bank high / RAM bank     value[1:0]
6000–7FFF                   Banking mode                 value[0]
```

```text
			  mode 0: ROM banking                mode 1: RAM banking
0000–3FFF     ROM bank 00                         ROM bank (high << 5)
4000–7FFF     ROM bank (high << 5 | low)          ROM bank low
A000–BFFF     RAM bank 0                           RAM bank high
```

`low` is the five-bit value written to `2000–3FFF`; `high` is the two-bit value written to `4000–5FFF`. Mode `0` is the reset state.

### Bank-zero rule

The switchable ROM window may not select bank numbers whose low five bits are zero. A requested bank `00`, `20`, `40`, 
or `60` becomes `01`, `21`, `41`, or `61` respectively.

```text
write 2000 = 00, high = 0  -> 4000–7FFF shows bank 01
write 2000 = 00, high = 1  -> 4000–7FFF shows bank 21 (mode 0)
write 2000 = 05, high = 2  -> 4000–7FFF shows bank 45 (mode 0)
```

Cartridges with fewer banks ignore unconnected bank-select lines. The implementation should derive valid capacity from 
the loaded ROM and avoid indexing past it; it must not rely only on a modulo operation that changes hardware-visible selection semantics.

## Address-level CPU examples

### Select ROM bank `0x25` in mode 0

```text
CPU write 6000 = 00       mode = 0
CPU write 2000 = 05       low  = 00101b
CPU write 4000 = 01       high = 01b

4000–7FFF selected bank = (01b << 5) | 00101b = 0x25
CPU read  4000            -> ROM[0x25 * 0x4000 + 0x0000]
CPU read  7FFF            -> ROM[0x25 * 0x4000 + 0x3FFF]
CPU read  0150            -> ROM[0x0000 * 0x4000 + 0x0150]
```

### Select RAM bank `2` in mode 1

```text
CPU write 0000 = 0A       enable external RAM
CPU write 6000 = 01       mode = 1
CPU write 4000 = 02       high = 2, therefore RAM bank = 2
CPU write A123 = 5A       -> RAM[2 * 0x2000 + 0x0123] = 5A
```

In this mode, `0000–3FFF` maps ROM bank `0x40`, subject to the cartridge's physical capacity, while `4000–7FFF` uses only `low`.

## Current code walkthrough

The snapshot below from [`src/memory_bus.cpp`](../../src/memory_bus.cpp) is a useful first implementation template. 
It keeps mapper state in `rom_bank_number`, `ram_bank_number`, `ram_enabled`, and `rom_ram_mode_select`.

### Control writes

```cpp
if (cart_info.type == CART_TYPE::MBC1) {
	if (addr >= 0x0000 && addr <= 0x1FFF) { // RAM Enable
		ram_enabled = (value & 0xF0) == 0x0A;
	}

	if (addr >= 0x2000 && addr <= 0x3FFF) {
		rom_bank_number = value & 0b00011111; // This is a 5-bit register
		const uint32_t rom_size = 32 * (1 << cartridge_header->rom_size);
		const uint8_t number_of_rom_banks = rom_size / 16;
		rom_bank_number = rom_bank_number % number_of_rom_banks;
	}

	if (addr >= 0x4000 && addr <= 0x5FFF) {
		if (rom_ram_mode_select) {
			ram_bank_number = value & 0b00000011; // This is a 2-bit register
		} else {
			rom_bank_number = (rom_bank_number & 0b00011111) | ((value & 0b00000011) << 5);
		}
	}

	if (addr >= 0x6000 && addr <= 0x7FFF) {
		rom_ram_mode_select = value & 1; // This is a 1-bit register
	}
}
```

The first range correctly recognizes the MBC1 RAM-enable command by checking the low nibble. The low-ROM-bank and 
high-register masks are also the correct widths. In mode 0, the high two bits are merged into `rom_bank_number`; 
in mode 1 they become `ram_bank_number`.

The code's `rom_size` calculation needs a header-size decoder rather than assuming every `rom_size` code follows `32 KiB << code`; 
special `0x52`–`0x54` codes exist. The calculated bank count is stored in `uint8_t`, which cannot represent all values 
needed by a general mapper implementation. Retain separate `low`, `high`, and `mode` registers, then calculate the selected 
bank at read time, so mode changes immediately affect both ROM windows.

### Switchable ROM read

```cpp
} else if (cart_info.type == CART_TYPE::MBC1) {
	const uint16_t ROM_BANK_SIZE = 0x4000; // 16k per ROM bank
	uint8_t rom_bank = rom_bank_number > 0 ? rom_bank_number : 1;
	const uint32_t offset = ROM_BANK_SIZE * (rom_bank - 1);
	return cartridge_data[offset + addr];
}
```

For a `4000–7FFF` address, this is algebraically equivalent to the conventional translation:

```text
(rom_bank - 1) * 0x4000 + addr
= rom_bank * 0x4000 + (addr - 0x4000)
```

For example, `rom_bank = 2`, `addr = 0x4123` yields `ROM[0x8123]`, byte `0x0123` of bank 2. The `rom_bank > 0 ? ... : 1` 
expression implements only the `00 -> 01` alias; it must be extended to aliases `20`, `40`, and `60` and to mode-dependent selection.

### External RAM read and write

```cpp
} else if (cart_info.type == CART_TYPE::MBC1) {
	const uint16_t bank_offset = ram_bank_number * 0x2000; // 8k per RAM bank
	const uint16_t eram_address = (addr - 0xA000) + bank_offset;
	return eram[eram_address];
}
```

```cpp
} else if (cart_info.type == CART_TYPE::MBC1) {
	const uint16_t RAM_BANK_SIZE = 0x2000; // 8k per ROM bank
	const uint32_t offset = RAM_BANK_SIZE * ram_bank_number;
	const uint16_t eram_address = offset + (addr - 0xA000);
	eram[eram_address] = value;
}
```

Both snippets use the correct 8 KiB bank offset. The write comment says “per ROM bank” but the calculation is for a RAM bank. 
They currently do not check `ram_enabled`, `cart_info.has_ram`, allocated RAM size, or the MBC1 mode before exposing a selected RAM bank.

## Implementation checklist

- Store separate low-ROM, high-bank, mode, and RAM-enable latches; reset them to hardware-appropriate values.
- On every ROM read, calculate the lower and upper window bank from the latches and mode.
- Apply forbidden-bank aliasing before calculating the upper-window offset.
- Reject disabled/nonexistent RAM reads and ignore writes according to the selected open-bus policy.
- Bound every translated ROM/RAM offset against actual loaded/allocation size.
- Do not combine this mapper's state with future mapper families; give each mapper its own state.

## Tests to add

1. Verify `00`, `20`, `40`, and `60` low-bank selections alias in the `4000–7FFF` window.
2. Verify upper-register writes affect the upper ROM window in mode 0 and the lower ROM/RAM windows in mode 1.
3. Verify RAM reads/writes fail or return the chosen open-bus value while RAM is disabled.
4. Verify each available RAM bank produces an independent `A000–BFFF` value in mode 1.
5. Verify a mode switch recalculates maps from latched register values rather than corrupting the low-bank register.

## Sources

- [GB Pan Docs: MBC1](https://gbdev.io/pandocs/MBC1.html)
- [GB Pan Docs: cartridge header](https://gbdev.io/pandocs/The_Cartridge_Header.html)
- [GBDev Wiki: Game Boy cartridge types](https://gbdev.io/wiki/articles/Gameboy_Cartridge_Types)
- [`src/memory_bus.cpp`](../../src/memory_bus.cpp), current repository implementation
- [`src/cart_type.h`](../../src/cart_type.h), current repository header-byte mapping
