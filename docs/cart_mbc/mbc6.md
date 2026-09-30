# MBC6

[Back to roadmap](../memory_bus_cart_mbc_roadmap.md)

## Header type

`20` is `MBC6 + RAM + Battery` in `cart_type.h`.

## Mapping model

MBC6 differs from the 16 KiB-window controllers: `4000–5FFF` and `6000–7FFF` are independently selected **8 KiB** ROM windows.

```text
CPU window     Mapping
0000–3FFF      fixed ROM region
4000–5FFF      selected 8 KiB ROM bank A
6000–7FFF      selected 8 KiB ROM bank B
A000–BFFF      selected 8 KiB SRAM bank
```

Pan Docs documents split control registers for each 8 KiB ROM bank:

```text
2000–27FF  ROM bank A low seven bits
2800–2FFF  ROM bank A high bit
3000–37FF  ROM bank B low seven bits
3800–3FFF  ROM bank B high bit
4000–4FFF  SRAM bank selection
0000–1FFF  SRAM enable
```

A correct translation is therefore:

```text
read 4000 -> ROM[bank_a * 0x2000 + 0x0000]
read 5FFF -> ROM[bank_a * 0x2000 + 0x1FFF]
read 6000 -> ROM[bank_b * 0x2000 + 0x0000]
read A123 -> SRAM[sram_bank * 0x2000 + 0x0123]
```

Do not reuse a mapper abstraction that assumes exactly one 16 KiB switchable ROM window.

## Flash and deferred behavior

MBC6 cartridges include flash memory and command behavior beyond ordinary ROM banking. Preserve the address/control boundary described by Pan Docs, but defer flash command sequencing, erase/program effects, and persistence. Treat programmed ROM contents as read-only loaded image data until a flash subsystem exists. Hardware details are comparatively less documented than MBC1–5; validate against the cited research before enabling write behavior.

## Implementation requirements

Store two independent eight-bit ROM-bank latches, RAM enable, and RAM bank. Allocate image storage from the file length rather than the repository's present 1 MiB fixed buffer, and use 8 KiB—not 16 KiB—offset calculations. Battery save handling is deferred.

## Tests

1. Pattern each 8 KiB ROM bank and prove `4000` and `6000` can show different banks simultaneously.
2. Verify high-bit writes select banks above `0x7F` when image capacity permits.
3. Verify SRAM banking and enable gating at `A000–BFFF`.
4. Verify flash-control writes do not corrupt loaded ROM before flash behavior is implemented.

## Sources

- [GB Pan Docs: MBC6](https://gbdev.io/pandocs/MBC6.html)
- [GBDev Wiki: Game Boy cartridge types](https://gbdev.io/wiki/articles/Gameboy_Cartridge_Types)
