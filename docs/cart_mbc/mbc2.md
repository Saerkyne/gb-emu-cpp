# MBC2

[Back to roadmap](../memory_bus_cart_mbc_roadmap.md)

## Header types

| Header byte | Description |
| --- | --- |
| `05` | MBC2 |
| `06` | MBC2 + battery |

MBC2 contains 512 × 4-bit RAM cells internally. It does not use a normal external RAM chip; battery persistence for type `06` is deferred.

## Registers and address decoding

MBC2 distinguishes commands with **address bit A8**, not with separate broad address blocks.

```text
CPU write address             Meaning
0000–1FFF, A8 = 0             RAM enable: value[3:0] == Ah
0000–1FFF, A8 = 1             ROM bank: value[3:0]
2000–3FFF, A8 = 1             ROM bank: value[3:0] (mirrored decode)
```

Typical emulation may decode the full `0000–3FFF` region as follows:

```text
if address bit 8 is 0: RAM enable latch
if address bit 8 is 1: four-bit ROM bank latch
```

The switchable `4000–7FFF` ROM window selects one of banks `01–0F`; bank `00` aliases to bank `01`.

```text
CPU write 2100 = 03  -> 4000–7FFF maps ROM bank 03
CPU write 2000 = 00  -> 4000–7FFF maps ROM bank 01
CPU read  4012       -> ROM[0x03 * 0x4000 + 0x0012]
```

## Internal RAM window

The CPU uses `A000–BFFF`, but only address bits A0–A8 select one of 512 nibbles. Addresses with the same low nine bits mirror each other.

```text
CPU address A000 -> RAM nibble 000
CPU address A1FF -> RAM nibble 1FF
CPU address A200 -> RAM nibble 000 (mirror)
```

Writes store `value[3:0]` only. Reads conventionally return `0xF0 | stored_nibble`:

```text
CPU write A123 = 5A  -> nibble_ram[0x123] = A
CPU read  A123       -> FA
```

When RAM is disabled, reads should return the emulator's chosen open-bus value (normally `FF`) and writes must be ignored.

## Implementation requirements

- Use `std::array<uint8_t, 512>` or a packed 256-byte nibble store; generic 8 KiB `eram` is the wrong model.
- Decode A8 from the **write address**, even if the value and broad range look valid.
- Mask ROM-bank writes with `0x0F`, map zero to one, and bounds-check the resulting 16 KiB ROM offset.
- Mask internal-RAM addresses with `0x01FF` and data with `0x0F`; set the read high nibble to `0xF`.
- Reset the selected ROM bank to 1 and RAM-enable latch to disabled.

The unfinished MBC2 branch in `memory_bus.cpp` already masks `rom_bank_number` with `0x0F` and recognizes zero-to-one aliasing, but it does not calculate or return the ROM offset and has no A8/RAM implementation.

## Tests to add

1. Write bank commands to `2000` and `2100` (A8 clear) and confirm they affect RAM enable, not bank selection.
2. Write a bank command to `2100` (A8 set) and verify a patterned bank at `4000`.
3. Verify `00` aliases to bank 1.
4. Verify `A000` and `A200` access the same nibble, with a `0xF` high read nibble.
5. Verify disabled RAM ignores writes.

## Sources

- [GB Pan Docs: MBC2](https://gbdev.io/pandocs/MBC2.html)
- [GBDev Wiki: Game Boy cartridge types](https://gbdev.io/wiki/articles/Gameboy_Cartridge_Types)
- [`src/memory_bus.cpp`](../../src/memory_bus.cpp), current incomplete MBC2 branch
