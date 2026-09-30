# Memory Bus Cartridge MBC Roadmap

This roadmap describes the cartridge-side portion of the Game Boy memory bus. It is based on the current implementation 
in [`src/memory_bus.cpp`](../src/memory_bus.cpp), the cartridge-type table in [`src/cart_type.h`](../src/cart_type.h), 
and the cartridge header layout in [`src/cart.h`](../src/cart.h).

The scope is **mapper decoding and bank selection only**. Battery-backed persistence, RTC progression, rumble output, 
camera capture, accelerometer input, EEPROM persistence, infrared, flash programming, and TAMA5 peripheral emulation are deferred. 
Registers that select those devices must still be decoded and documented so later work has a defined integration point.

## CPU cartridge address windows

```text
CPU address       Cartridge meaning
0000–3FFF         Fixed or mapper-selected lower ROM window
4000–7FFF         Mapper-selected upper ROM window
0000–1FFF writes  Common RAM/device enable register range
2000–3FFF writes  Common low ROM-bank register range
4000–5FFF writes  Common high ROM-bank/RAM-bank/device-select range
6000–7FFF writes  Common mode/latch/special-control range
A000–BFFF         External RAM or mapper-selected peripheral/device window
```

The exact meaning of a range is mapper-specific. Treat writes to `0000–7FFF` as cartridge control writes, not writes to 
the ROM byte array.

## Current implementation assessment

`memory_bus.cpp` currently has shared global mapping state:

```text
rom_bank_number, ram_bank_number, ram_enabled, rom_ram_mode_select
```

It provides ROM-only and partial MBC1 routing, plus an unfinished MBC2 read branch. The ROM-only and MBC1 pages contain 
annotated snapshots of that code. They form the documentation template for later controllers:

1. Define mapper-owned state and its reset values.
2. Decode each CPU control-write range.
3. Translate CPU ROM reads into validated cartridge-image offsets.
4. Route `A000–BFFF` reads/writes to RAM or the selected device.
5. Record hardware differences, disabled-device behavior, size limits, and tests.

### Capacity prerequisites

The current source declares a 1 MiB ROM buffer in `cart.h` and 32 KiB of external RAM in `memory_bus.h`. Those capacities 
are not sufficient for all controller families in `cart_type.h`:

- MBC5 supports up to 8 MiB of ROM and up to 128 KiB of RAM.
- MBC6 supports a larger ROM organization than the current 1 MiB image buffer.
- MBC2 needs 512 × 4-bit internal RAM, not generic 8 KiB external RAM.
- MBC7 needs serial EEPROM state rather than generic external RAM.
- Camera, HuC3, HuC1, and TAMA5 can expose device registers through the external-device window.

Before implementing those mappers, replace fixed backing arrays with allocations sized from the ROM file and validated 
header metadata. Maintain mapper state per loaded cartridge rather than as process-wide globals.

## Suggested implementation sequence

1. Correct and test ROM-only plus MBC1 behavior using the walkthrough pages.
2. Refactor cartridge storage and mapper state behind a mapper interface or equivalent cartridge-owned state object.
3. Implement MBC2, then MBC3 RAM banking with RTC registers stubbed at the documented boundary.
4. Implement MBC5, including a capacity-safe ROM image and RAM allocation.
5. Add MMM01, HuC1, and HuC3 after the common MBC1-style infrastructure is stable.
6. Implement MBC6, MBC7, Pocket Camera, and TAMA5 as special-device mappers, with device behavior initially stubbed but 
1. address decoding exact.

## Mapper reference pages

| Family | Page | Header type bytes in current table |
| --- | --- | --- |
| ROM-only / no MBC | [no_mbc.md](cart_mbc/no_mbc.md) | `00`, `08`, `09` |
| MBC1 | [mbc1.md](cart_mbc/mbc1.md) | `01`–`03` |
| MBC2 | [mbc2.md](cart_mbc/mbc2.md) | `05`–`06` |
| MBC3 | [mbc3.md](cart_mbc/mbc3.md) | `0F`–`13` |
| MBC5 | [mbc5.md](cart_mbc/mbc5.md) | `19`–`1E` |
| MBC6 | [mbc6.md](cart_mbc/mbc6.md) | `20` |
| MBC7 | [mbc7.md](cart_mbc/mbc7.md) | `22` |
| MMM01 | [mmm01.md](cart_mbc/mmm01.md) | `0B`–`0D` |
| Pocket Camera | [pocket_camera.md](cart_mbc/pocket_camera.md) | `FC` |
| Bandai TAMA5 | [bandai_tama5.md](cart_mbc/bandai_tama5.md) | `FD` |
| HuC3 | [huc3.md](cart_mbc/huc3.md) | `FE` |
| HuC1 | [huc1.md](cart_mbc/huc1.md) | `FF` |

## Test strategy

For every mapper, test control writes through `memory_bus_write` and reads through `memory_bus_read`; do not test only 
internal state. Build synthetic ROM images where each 16 KiB bank contains its bank number, then assert the bytes visible 
at `0000`, `3FFF`, `4000`, and `7FFF`. Fill each RAM bank or device register with distinct values and test `A000` and `BFFF`, 
including disabled-device reads/writes. Add boundary tests for forbidden banks, maximum supported banks, header sizes, 
and out-of-range offsets.

Use accepted mapper test ROMs and, where possible, compare behavior against a hardware-validated emulator. Feature 
behavior outside the current scope should have tests for register routing and deterministic placeholder responses only 
until its subsystem is implemented.

## Sources

- [GB Pan Docs: Cartridge header](https://gbdev.io/pandocs/The_Cartridge_Header.html)
- [GB Pan Docs: MBC1](https://gbdev.io/pandocs/MBC1.html)
- [GB Pan Docs: MBC2](https://gbdev.io/pandocs/MBC2.html)
- [GB Pan Docs: MBC3](https://gbdev.io/pandocs/MBC3.html)
- [GB Pan Docs: MBC5](https://gbdev.io/pandocs/MBC5.html)
- [GB Pan Docs: MBC6](https://gbdev.io/pandocs/MBC6.html)
- [GB Pan Docs: MBC7](https://gbdev.io/pandocs/MBC7.html)
- [GBDev Wiki: Game Boy cartridge types](https://gbdev.io/wiki/articles/Gameboy_Cartridge_Types)
- [Current repository `memory_bus.cpp`](../src/memory_bus.cpp), `memory_bus.h`, `cart.h`, and `cart_type.h` (implementation-specific observations)
