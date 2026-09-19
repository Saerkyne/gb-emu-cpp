#include "emulator_core.h"

#include "cart.h"
#include "cpu.h"
#include "timer.h"


uint32_t core_clock_counter = 0; // Global clock counter for the emulator core
bool core_quit_requested = false; // Flag to indicate if the emulator core should quit

int core_init() {
	const char* tetris_path = "C:\\Users\\jthubbard\\OneDrive - Randolph Community College\\Documents\\Coding\\gb-emu-cpp\\rom\\cpu_instrs.gb";

	if (!cart_load(tetris_path)) {
		return -1;
	}

	cart_print_info();

	return 0;
}

void core_run() {
	cpu_reset();

	while (!core_quit_requested) {
		cpu_tick();
	}
}

void core_shutdown() {
	
}

void core_advance_cpu_clocks(uint8_t clocks) {
	timer_advance_clocks(clocks);
	
	core_clock_counter += clocks;
}