# MBC3

[Back to roadmap](../memory_bus_cart_mbc_roadmap.md)

## Header types

`0F` (RTC+battery), `10` (RAM+RTC+battery), `11` (MBC3), `12` (MBC3+RAM), and `13` (MBC3+RAM+battery) use MBC3.

## Address map

```text
Write range    Register                         Bits / values
0000–1FFF      RAM and RTC enable               low nibble = A enables
2000–3FFF      upper ROM bank                   value[6:0], 00 aliases to 01
4000–5FFF      RAM bank / RTC register select   00–03: RAM; 08–0C: RTC
6000–7FFF      RTC latch command                write 00, then 01
A000–BFFF      selected RAM bank or RTC register
```

ROM `0000–3FFF` is fixed bank 0. `4000–7FFF` maps the selected 16 KiB bank:

```text
write 2000 = 45; read 4010 -> ROM[0x45 * 0x4000 + 0x10]
```

With enable active, selector `00–03` maps 8 KiB RAM banks:

```text
write 0000 = 0A; write 4000 = 02; write A100 = 7C
-> RAM[2 * 0x2000 + 0x100] = 7C
```

Selectors `08` through `0C` route `A000–BFFF` to RTC seconds, minutes, hours, day-low, and day-high/halt/carry registers rather than RAM. A latch occurs only on the `00` then `01` transition written to `6000–7FFF`; reads use the latched RTC snapshot.

## Deferred RTC behavior

Decode the selector and latch sequence now. Defer clock progression, halt/carry semantics, latched-register storage, wall-clock reconciliation, and battery persistence. Until then, return deterministic placeholder values for selected RTC registers and ignore/record writes consistently; do not accidentally route them into RAM.

## Implementation requirements

Keep a seven-bit ROM-bank latch, a four-bit RAM/RTC selector, an enable latch, and prior latch-write state. RAM selection is not a ROM/RAM mode like MBC1. Bound checked ROM/RAM offsets must be derived from actual storage allocation.

## Tests

1. Confirm `00` ROM bank aliases to 1.
2. Confirm RAM banks `00–03` remain independent.
3. Confirm disabled RAM/RTC accesses are blocked.
4. Confirm selectors `08–0C` invoke RTC routing, not RAM routing.
5. Confirm `01` without a preceding `00` does not latch.

## Sources

- [GB Pan Docs: MBC3](https://gbdev.io/pandocs/MBC3.html)
- [GB Pan Docs: cartridge header](https://gbdev.io/pandocs/The_Cartridge_Header.html)
- [GBDev Wiki: Game Boy cartridge types](https://gbdev.io/wiki/articles/Gameboy_Cartridge_Types)
