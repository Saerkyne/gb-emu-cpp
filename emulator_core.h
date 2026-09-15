#pragma once
#include <stdint.h>

extern uint32_t core_clock_counter; // Global clock counter for the emulator core

int core_init(); // Initializes the emulator core
void core_run(); // Runs the emulator core
void core_shutdown(); // Shuts down the emulator core

void core_advance_cpu_clocks(uint8_t clocks); // Advances the CPU clock by a specified number of cycles