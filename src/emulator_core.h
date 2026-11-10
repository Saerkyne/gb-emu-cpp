#pragma once
#include "gfx.h"
#include <stdint.h>

extern gfx_context* gfx_debug_ctx;
extern gfx_context* gfx_render_ctx;
extern uint32_t core_clock_counter; // Global clock counter for the emulator core

// Test Mode block
const int CORE_EXIT_OK = 0;
const int CORE_EXIT_TEST_FAILED = 1;
const int CORE_EXIT_TEST_NO_RESULT = 2;
const int CORE_EXIT_NO_ROM = 3;
const int CORE_EXIT_BAD_ARGS = 4;

extern bool test_mode; // Flag to indicate if the emulator is running in test mode
extern int core_exit_code; // Exit code for the emulator core, used in test mode

void core_report_test_result(bool passed); // Reports the result of a test case, sets the exit code accordingly
// End Test Mode Block

int core_init(); // Initializes the emulator core
void core_run(); // Runs the emulator core
void core_shutdown(); // Shuts down the emulator core
void core_on_frame();

void core_advance_cpu_clocks(uint8_t clocks); // Advances the CPU clock by a specified number of cycles
void core_request_rom_load(const char* path); // Requests loading a ROM from the specified path at the next safe point
