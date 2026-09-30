# MBC7

[Back to roadmap](../memory_bus_cart_mbc_roadmap.md)

## Header type

`22` is `MBC7 + RAM + Battery + Accelerometer`. Its nonvolatile storage is serial EEPROM, not ordinary banked SRAM.

## ROM control

MBC7 uses the familiar cartridge-control ranges for RAM/device enable and an upper-ROM bank register:

```text
0000–1FFF writes  enable the external device path
2000–3FFF writes  select the switchable 16 KiB ROM bank
4000–7FFF          mapper-specific control/reserved behavior
0000–3FFF reads    fixed ROM; 4000–7FFF reads selected ROM
```

Keep the ROM-bank latch separate from MBC1/MBC5 because device access is not `ram_bank * 0x2000`.

## `A000–BFFF`: serial device, not RAM

MBC7 exposes accelerometer registers and a 93LC56-compatible serial EEPROM interface through the external-device window. The EEPROM control port is addressed at `A080`; software drives serial input, clock, and chip-select control bits by writing there, and reads serial output from that port. The protocol is bit-serial:

```text
CPU writes A080: CS=1, DI=next command/data bit
CPU writes A080: CLK rising edge shifts/samples that bit
(repeat command/address/data bit sequence)
CPU reads  A080: observe EEPROM DO bit while CS is selected
```

The EEPROM command stream contains start, opcode, address, and data bits; it is not an 8 KiB byte-addressed RAM array. The accelerometer register addresses/control are documented by Pan Docs and must route to an input-provider boundary rather than generic RAM.

## Deferred features

Decode the enable and `A080` EEPROM path now. Defer EEPROM persistence, write-cycle timing, accelerometer host input/calibration, and battery save files. A first mapper-only implementation may hold a deterministic in-memory EEPROM model, but it must preserve serial edge semantics; returning generic `eram[addr - A000]` will not run MBC7 software correctly.

## Implementation requirements

- Represent EEPROM data and serial state separately: chip-select, clock history, DI, DO, command shift register, address, and busy/write state.
- Route only documented device addresses; define open-bus behavior for other `A000–BFFF` addresses.
- On a clock transition, execute one EEPROM protocol state-machine step.
- Delegate accelerometer reads to a deferred input interface with stable placeholder values until implemented.

## Tests

1. Verify selected ROM banks in `4000–7FFF`.
2. Verify a minimal EEPROM read command shifts expected DO bits at `A080`.
3. Verify a clock write without selected chip does not change EEPROM state.
4. Verify accelerometer addresses do not alias EEPROM or generic RAM.

## Sources

- [GB Pan Docs: MBC7](https://gbdev.io/pandocs/MBC7.html)
- [GBDev Wiki: Game Boy cartridge types](https://gbdev.io/wiki/articles/Gameboy_Cartridge_Types)
