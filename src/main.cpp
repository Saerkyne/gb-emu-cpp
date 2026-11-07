#include <cstdio>

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "emulator_core.h"

int main(int argc, char* argv[]) 
{
	int error = core_init();
	if (error != 0)
	{
		return error;
	}

	// A path here comes from the command line, a ROM dropped on the executable, 
	// or the "open with" context menu in the file explorer. If a path is provided, we will load it immediately.
	if (argc > 1)
	{
		if (argc > 2)
		{
			printf("Warning, %d files were given; loading the first one.\n", argc - 1);
		}
		core_request_rom_load(argv[1]);
	}

	core_run();

	core_shutdown();

	return 0;
}