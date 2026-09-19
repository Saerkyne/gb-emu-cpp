#include <iostream>

#define SDL_MAIN_HANDLED
#include <SDL3\SDL.h>
#include <SDL3\SDL_main.h>

#include "emulator_core.h"

int SDLCALL gb_emu_runapp_callback(int argc, char* argv[]);

int main(int argc, char* argv[]) {
	return SDL_RunApp(argc, argv, gb_emu_runapp_callback, NULL);
}

int SDLCALL gb_emu_runapp_callback(int argc, char* argv[]) {
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		return -1;
	}
	
	int error = core_init();
	if (error != 0) {
		return error;
	}

	core_run();

	core_shutdown();

	SDL_Quit();

	return 0;
}

