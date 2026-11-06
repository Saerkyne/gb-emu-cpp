#include <iostream>

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "emulator_core.h"

int SDLCALL gb_emu_runapp_callback(int argc, char* argv[]);

int main(int argc, char* argv[]) 
{
	return SDL_RunApp(argc, argv, gb_emu_runapp_callback, NULL);
}

int SDLCALL gb_emu_runapp_callback(int argc, char* argv[]) 
{	
	int error = core_init();
	if (error != 0) 
	{
		return error;
	}

	char* rom_path = "/home/cal/gb-emu-cpp/rom/Tetris.gb";
	core_request_rom_load(rom_path);

	core_run();

	core_shutdown();

	return 0;
}

