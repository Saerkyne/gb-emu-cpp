#pragma once
#include "gfx.h"
#include <stdint.h>

extern gfx_context* gfx_debug_ctx;
extern gfx_context* gfx_render_ctx;
extern uint32_t core_clock_counter; // Global clock counter for the emulator core

int core_init(); // Initializes the emulator core
void core_run(); // Runs the emulator core
void core_shutdown(); // Shuts down the emulator core
void core_on_frame();

void core_advance_cpu_clocks(uint8_t clocks); // Advances the CPU clock by a specified number of cycles
void core_request_rom_load(const char* path); // Requests loading a ROM from the specified path at the next safe point
