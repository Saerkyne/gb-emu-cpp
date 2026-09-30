# Bandai TAMA5

[Back to roadmap](../memory_bus_cart_mbc_roadmap.md)

## Header type

`FD` identifies the Bandai TAMA5 cartridge.

## Mapping model and confidence boundary

TAMA5 is a special Bandai cartridge controller with ROM banking plus a command/data interface for its clock/peripheral hardware. It is **not** compatible with an ordinary `A000–BFFF` RAM-bank calculation. Public hardware documentation is incomplete compared with MBC1–5, so implementation should begin with a protocol state machine and validated test traces, not inferred generic RAM behavior.

At the CPU level, the mapper must distinguish:

```text
0000–7FFF writes  ROM-bank and TAMA5 command/register selection
4000–7FFF reads   selected switchable ROM bank
A000–BFFF         TAMA5 command/data response window where documented
```

The controller sequence is register-oriented: CPU writes select a command/register and transfer a nibble/byte of data; later reads return the selected device response. Model this as explicit latches:

```text
rom_bank
command_or_register_select
input_data_latch
output_data_latch
protocol_state
```

This prevents later clock/peripheral emulation from being entangled with generic external RAM.

## Deferred features

Clock/calendar progression, Tamagotchi peripheral behavior, battery persistence, and any serial/protocol timing outside mapper decode are deferred. The first implementation should still recognize documented command/address paths and return deterministic values for unavailable device data. Clearly record each verified command from a test ROM or trace before expanding behavior.

## Implementation requirements

- Isolate TAMA5 in its own mapper class/state; do not inherit MBC1 register assumptions.
- Use traceable command/data latches, with reads routed by selected command rather than raw ERAM index.
- Guard all ROM-bank offsets against loaded image size.
- Treat unverified command values as open bus/no-op until backed by documentation or cartridge traces.

## Tests

1. Verify known ROM-bank selection with a patterned ROM.
2. Record and assert the address/value sequence used by a known TAMA5 title during startup.
3. Verify command selection changes device-response routing without altering ROM data.
4. Verify unknown commands remain deterministic and cannot access generic RAM.

## Sources

- [GBDev Wiki: Game Boy cartridge types](https://gbdev.io/wiki/articles/Gameboy_Cartridge_Types)
- [GB Pan Docs](https://gbdev.io/pandocs/), cartridge-header/type context

> Research note: before implementing individual TAMA5 command values, add a source that documents the exact register protocol or capture a reproducible trace from verified hardware/emulator research. This page intentionally does not present unverified register values as fact.
