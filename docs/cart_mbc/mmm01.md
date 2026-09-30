# MMM01

[Back to roadmap](../memory_bus_cart_mbc_roadmap.md)

## Header types

`0B` (MMM01), `0C` (MMM01 + RAM), and `0D` (MMM01 + RAM + battery) use this multicart-oriented controller.

## Mapping model

MMM01 is MBC1-like only **after initialization**. It powers on in a special boot mapping so a multicart menu/initialization program is visible, then an initialization write locks a base-ROM/base-RAM selection. Subsequent bank writes use that locked base plus MBC1-style bank bits.

```text
state: boot mapping  -- initialization command -->  locked normal mapping
									  (one-way)
```

The important implementation distinction is state: MBC1 has no startup lock state, while MMM01 must retain its initial mapping until the documented lock transition occurs. Do not map later control writes as ordinary MBC1 registers before that transition.

## CPU ranges

MMM01 uses cartridge control writes within `0000–7FFF` and RAM/device access at `A000–BFFF`. Pan Docs assigns the precise initial bank, base-register, mode, and lock semantics; implement the register state exactly from that table because games can depend on the pre-lock mapping.

After lock-in, calculate addresses conceptually as:

```text
physical ROM bank = locked ROM base + normal selected bank
physical RAM bank = locked RAM base + selected RAM bank
ROM offset = physical ROM bank * 0x4000 + window-relative address
RAM offset = physical RAM bank * 0x2000 + (addr - 0xA000)
```

The arithmetic must be bounded against actual image/RAM capacity. RAM enable and any battery persistence remain separate concerns.

## Implementation requirements

- Model a boot/locked state plus every documented MMM01 bank/base/mode latch.
- Make the lock transition irreversible until cartridge reset.
- Keep the controller separate from MBC1 even if post-lock register behavior appears similar.
- Test with a patterned multicart image: fixed lower-window bytes must change only at the documented lock point.

## Deferred features

Battery persistence for type `0D` is deferred. This does not defer RAM-enable, bank selection, or lock-state routing.

## Sources

- [GB Pan Docs: MMM01](https://gbdev.io/pandocs/MMM01.html)
- [GBDev Wiki: Game Boy cartridge types](https://gbdev.io/wiki/articles/Gameboy_Cartridge_Types)

> Hardware documentation for MMM01 is less complete than the common Nintendo MBCs. Validate exact reset and lock values against the cited primary documentation and a known multicart test image before marking the mapper complete.
