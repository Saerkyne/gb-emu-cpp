#include "emulator_core.h"

#include "cart.h"
#include "cpu.h"


uint32_t core_clock_counter = 0; // Global clock counter for the emulator core
bool core_quit_requested = false; // Flag to indicate if the emulator core should quit

int core_init() {
	const char* tetris_path = "C:\\Users\\jthubbard\\OneDrive - Randolph Community College\\Documents\\Coding\\test\\rom\\Tetris.gb";

	if (!cart_load(tetris_path)) {
		return -1;
	}

	cart_print_info();

	return 0;
}

void core_run() {
	cpu_reset();

	while (!core_quit_requested) {
		cpu_fetch();
		if (!cpu_execute()) {
			core_quit_requested = true;
		}
	}
}

void core_shutdown() {
	
}

void core_advance_cpu_clocks(uint8_t clocks) {
	core_clock_counter += clocks;
}