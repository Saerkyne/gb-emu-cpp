# ROM-only cartridges (`NO_MBC`)

[Back to roadmap](../memory_bus_cart_mbc_roadmap.md)

## Header types

| Header byte | `cart_type.h` description |
| --- | --- |
| `00` | ROM only |
| `08` | ROM + RAM |
| `09` | ROM + RAM + battery |

Battery persistence is outside this document's scope. The `08` and `09` types have no bank controller: RAM, if fitted, remains permanently visible.

## Hardware model

A ROM-only cartridge directly exposes its ROM in the CPU cartridge-ROM space. There are no controller registers, bank numbers, RAM-enable latch, or mode bits.

```text
CPU                       Cartridge storage
0000–3FFF  ----------->   ROM offset 0000–3FFF
4000–7FFF  ----------->   ROM offset 4000–7FFF
A000–BFFF  ----------->   fixed external RAM offset 0000–1FFF (only when RAM is fitted)
```

The CPU must not assume every `NO_MBC` type has RAM: type `00` has none. A robust implementation must allocate and expose RAM only when `cart_info.has_ram` is true and the header RAM-size code is valid.

## Current code walkthrough

The following repository snapshot from [`src/memory_bus.cpp`](../../src/memory_bus.cpp) is the current ROM-only implementation. It is useful as the simplest template for later mappers: recognize a cart address range, select backing storage, and calculate the cartridge-relative offset.

### ROM reads

```cpp
if (addr >= 0x0000 && addr <= 0x3FFF) { // Read from ROM bank 00
	return cartridge_data[addr];
}

if (addr >= 0x4000 && addr <= 0x7FFF) { // Read from ROM bank 01~NN
	if (cart_info.type == CART_TYPE::NO_MBC) {
		return cartridge_data[addr]; // For no MBC cartridges
	}
}
```

`cartridge_data[addr]` is direct CPU-address-to-ROM-offset translation. This works because a ROM-only image has no selected bank. The lower (`0000–3FFF`) and upper (`4000–7FFF`) windows are both fixed. For a 32 KiB image, address `4000` selects ROM byte `0x4000`; address `7FFF` selects ROM byte `0x7FFF`.

### External RAM reads and writes

```cpp
if (addr >= 0xA000 && addr <= 0xBFFF) { // External RAM
	if (cart_info.type == CART_TYPE::NO_MBC) {
		const uint16_t eram_address = (addr - 0xA000);
		return eram[eram_address];
	}
}
```

```cpp
if (addr >= 0xA000 && addr <= 0xBFFF) { // ERAM
	if (cart_info.type == CART_TYPE::NO_MBC) {
		eram[addr - 0xA000] = value;
	}
}
```

The subtraction translates the CPU window to the RAM-buffer index:

```text
CPU write A000, value 5A  -> eram[0000] = 5A
CPU write BFFF, value C3  -> eram[1FFF] = C3
CPU read  A000            -> eram[0000]
```

There is no bank offset because ROM-only cartridges do not bank RAM. Unlike MBC1, no control write can enable or disable access.

## Current implementation gaps

The snapshot is a clear baseline, but future refactoring should address these differences from hardware:

- It exposes `eram` for type `00`, even though the header says that type has no RAM.
- It assumes a full 8 KiB visible RAM window. Some ROM-only cartridges use smaller RAM sizes, so accesses beyond allocated RAM require defined open-bus behavior rather than an out-of-bounds access.
- It does not validate that `addr` is inside the loaded ROM image.
- Battery saves are not loaded or persisted; that is intentionally deferred.

## Template for a later mapper

For a controller with banking, retain this structure but replace the direct mapping with selected-bank translation:

```text
selected ROM offset = selected_bank * bank_size + (addr - window_start)
selected RAM offset = selected_ram_bank * 0x2000 + (addr - 0xA000)
```

The mapper page must then define the control writes that set `selected_bank`, whether bank zero is legal, and when external RAM/device access is enabled.

## Tests to add

1. For type `00`, confirm `0000`, `3FFF`, `4000`, and `7FFF` return the corresponding image bytes.
2. For types `08` and `09`, write/read `A000` and `BFFF` and verify no bank selection changes their values.
3. Confirm type `00` reads from `A000–BFFF` follow the chosen no-RAM/open-bus policy after storage metadata is introduced.
4. Use a 32 KiB patterned test ROM so fixed-window behavior cannot accidentally look like banked behavior.

## Sources

- [GB Pan Docs: cartridge header](https://gbdev.io/pandocs/The_Cartridge_Header.html)
- [GBDev Wiki: Game Boy cartridge types](https://gbdev.io/wiki/articles/Gameboy_Cartridge_Types)
- [`src/memory_bus.cpp`](../../src/memory_bus.cpp), current repository implementation
- [`src/cart_type.h`](../../src/cart_type.h), current repository header-byte mapping
