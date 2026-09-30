# MBC5

[Back to roadmap](../memory_bus_cart_mbc_roadmap.md)

## Header types

`19`–`1B` are normal MBC5 variants; `1C`–`1E` add rumble. RAM and battery status varies by type byte in `cart_type.h`.

## Address map

```text
Write range    Register                    Bits used
0000–1FFF      RAM enable                  low nibble = A enables
2000–2FFF      ROM bank low                8 bits
3000–3FFF      ROM bank high               value[0] (bit 8)
4000–5FFF      RAM bank / rumble control   normal: value[3:0]
A000–BFFF      selected RAM bank
```

Unlike MBC1–3, ROM bank zero is valid. The upper ROM window therefore selects banks `000–1FF`.

```text
write 2000 = 34; write 3000 = 01
selected bank = 0x134
read 4012 -> ROM[0x134 * 0x4000 + 0x12]
```

Normal RAM banking uses a four-bit index:

```text
write 0000 = 0A; write 4000 = 0C; write A002 = 55
-> RAM[0x0C * 0x2000 + 2] = 55
```

Rumble cartridges repurpose bit 3 of the `4000–5FFF` value for the motor. Bits `0–2` select only RAM banks `0–7`; bit 3 must be decoded as deferred rumble output, not treated as part of the RAM bank.

## Implementation requirements

Use a 16-bit value for the nine-bit ROM index. Allocate ROM from loaded file size: MBC5 supports up to 8 MiB, exceeding the repository's present 1 MiB `cartridge_data`. Allocate RAM from header capacity (up to 128 KiB), check the RAM-enable latch and `has_ram`, and bounds-check every offset. Separate rumble state from RAM-bank state.

## Tests

1. Verify banks `000`, `0FF`, `100`, and `1FF` at `4000`.
2. Verify bank zero remains selectable.
3. Verify RAM banks `0–F` on non-rumble types.
4. Verify a rumble write with bit 3 set does not select RAM bank `8`.
5. Verify disabled/nonexistent RAM is inaccessible.

## Sources

- [GB Pan Docs: MBC5](https://gbdev.io/pandocs/MBC5.html)
- [GB Pan Docs: cartridge header](https://gbdev.io/pandocs/The_Cartridge_Header.html)
- [GBDev Wiki: Game Boy cartridge types](https://gbdev.io/wiki/articles/Gameboy_Cartridge_Types)
