# Pocket Camera

[Back to roadmap](../memory_bus_cart_mbc_roadmap.md)

## Header type

`FC` identifies the Pocket Camera cartridge.

## Mapping model

Pocket Camera uses MBC-style ROM banking and a RAM-bank selector, but one selector value redirects `A000–BFFF` from save RAM to camera registers.

```text
0000–1FFF writes  RAM/camera enable
2000–3FFF writes  switchable ROM-bank register
4000–5FFF writes  RAM-bank or camera-register selection
A000–BFFF         selected save RAM bank or camera register block
```

For ordinary RAM-bank values, use the normal external-RAM translation:

```text
RAM offset = selected_ram_bank * 0x2000 + (CPU address - 0xA000)
```

The camera selector causes the same CPU window to access camera control/status registers instead. Software therefore uses an address-level sequence like:

```text
write 4000 = camera-register selector
write A000 = capture/control value
read  A000 = camera status/control readback
write 4000 = RAM bank 0
read  A000 = saved image/work RAM byte
```

Exact selector values and register bit meanings must follow Pan Docs; do not expose camera registers as generic RAM.

## Deferred camera behavior

Route camera register accesses now, but defer sensor input, exposure/capture timing, image conversion, and save persistence. A mapper-first implementation may provide deterministic status and a stable synthetic image source behind a dedicated camera-device interface. It must preserve register selection and capture-command boundaries for later replacement.

## Implementation requirements

Keep ROM bank, RAM/device enable, and RAM-or-camera selector state. Branch `A000–BFFF` accesses before computing a RAM offset. Allocate RAM from the cartridge's documented capacity and bounds-check it. Camera register storage must not overlap saved image RAM.

## Tests

1. Verify a selected ROM bank at `4000–7FFF`.
2. Verify two save RAM banks remain independent.
3. Verify the camera selector changes `A000` routing from RAM to camera state.
4. Verify camera control writes do not modify save RAM and vice versa.

## Sources

- [GB Pan Docs: Pocket Camera](https://gbdev.io/pandocs/Camera.html)
- [GBDev Wiki: Game Boy cartridge types](https://gbdev.io/wiki/articles/Gameboy_Cartridge_Types)
