# HuC1

[Back to roadmap](../memory_bus_cart_mbc_roadmap.md)

## Header type

`FF` identifies HuC1 + RAM + battery.

## MBC1-compatible banking

HuC1 uses the MBC1-style control structure for ordinary ROM and RAM mapping:

```text
0000–1FFF      RAM/IR enable control
2000–3FFF      five-bit low ROM-bank register
4000–5FFF      two-bit high ROM-bank or RAM-bank register
6000–7FFF      ROM-banking/RAM-banking mode
0000–3FFF      fixed or high-bank-selected lower ROM window
4000–7FFF      selected upper ROM window
A000–BFFF      selected RAM bank or infrared register path
```

Use the MBC1 equations and forbidden-bank rules documented in [MBC1](mbc1.md) for ordinary ROM/RAM operation:

```text
upper ROM bank (mode 0) = high << 5 | low
RAM bank (mode 1)       = high
RAM offset              = RAM bank * 0x2000 + (addr - 0xA000)
```

## Infrared routing

HuC1 adds infrared-related behavior that shares cartridge-side control/device handling. The mapper must distinguish IR-selected accesses from ordinary external RAM; do not assume every `A000–BFFF` access is save RAM after a HuC1 control write. The exact IR register bits and electrical behavior must follow the cited HuC1 documentation.

```text
ordinary selector -> A000–BFFF accesses selected RAM
IR selector       -> relevant cartridge access reaches IR state/input
```

## Deferred features

Implement MBC1-compatible bank selection and the routing decision first. Defer host IR transmission/reception, modulation/timing, external-device linking, and battery persistence. Expose deterministic IR input/output placeholder state through a separate interface so MBC1 RAM code remains unchanged when IR emulation is added.

## Implementation requirements

- Prefer HuC1-specific mapper state that reuses tested MBC1 bank calculations rather than tagging MBC1 with IR special cases.
- Retain separate low/high/mode latches and apply MBC1 zero-bank aliases.
- Gate RAM/device accesses using the HuC1 enable behavior.
- Route IR-selected accesses before calculating an ERAM index.

## Tests

1. Reuse all MBC1 ROM-bank alias and mode tests.
2. Verify RAM-bank switching while not in IR mode.
3. Verify the IR-selected address path cannot overwrite save RAM.
4. Verify stable placeholder IR reads while IR emulation is deferred.

## Sources

- [GB Pan Docs: HuC1](https://gbdev.io/pandocs/HuC1.html)
- [GB Pan Docs: MBC1](https://gbdev.io/pandocs/MBC1.html)
- [GBDev Wiki: Game Boy cartridge types](https://gbdev.io/wiki/articles/Gameboy_Cartridge_Types)
