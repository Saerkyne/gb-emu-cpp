# HuC3

[Back to roadmap](../memory_bus_cart_mbc_roadmap.md)

## Header type

`FE` identifies HuC3.

## Mapping model

HuC3 provides ROM banking, external RAM banking, and a command/data interface for its clock/infrared-capable hardware. It resembles MBC3 in broad cartridge windows, but it must not be treated as MBC3 RTC-register selection.

```text
0000–1FFF writes  RAM/device enable
2000–3FFF writes  switchable ROM-bank selection
4000–5FFF writes  RAM-bank or HuC3 command selection
A000–BFFF         selected RAM bank or HuC3 data/command response
```

For a RAM selection, calculate the usual banked-RAM address:

```text
RAM offset = selected_ram_bank * 0x2000 + (addr - 0xA000)
```

For a device-command selection, route `A000–BFFF` to HuC3 protocol state instead. Software issues commands by choosing a mode/command in the control range and then exchanges command data through the external-device window. This is the key mapper requirement: device-selected accesses must not read or write the save-RAM byte at the same CPU address.

## Deferred IR/clock behavior

Decode selection and command/data routing now. Defer RTC progression, calendar interpretation, infrared carrier/input behavior, persistent clock state, and battery saves. Use a dedicated HuC3 device interface with deterministic placeholder responses until those systems exist.

## Implementation requirements

Keep separate ROM-bank, RAM-bank, enable, command-select, and protocol-state values. Document every verified HuC3 command before implementing it; hardware information is less complete than MBC3. Bound ROM/RAM accesses and define open-bus/no-op behavior for disabled or unimplemented device commands.

## Tests

1. Verify patterned switchable ROM banking at `4000–7FFF`.
2. Verify ordinary RAM banks are independent.
3. Verify a device-selected `A000` access does not change selected RAM.
4. Verify disabled device/RAM accesses are blocked consistently.

## Sources

- [GB Pan Docs: HuC3](https://gbdev.io/pandocs/HuC3.html)
- [GBDev Wiki: Game Boy cartridge types](https://gbdev.io/wiki/articles/Gameboy_Cartridge_Types)
