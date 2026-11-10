#include <cstdio>
#include <cstring>
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
	test_mode = false;

	const char* arg;
	const char* rompath = "/path/to/rom.gb";
	for (int i = 1; i < argc; ++i)
	{
		arg = argv[i];
		if ((strcmp(arg,"-t")) || (strcmp(arg,"--test-mode")))
		{
			test_mode = true;
		}
		if ((strcmp(arg,"-r")) || (strcmp(arg,"--rom")))
		{
			if (i + 1 < argc)
			{
				rompath = argv[i++];
			}
			else
			{
				printf("--rom requires a path argument.\n");
				return(1);
			}
		}
	}

	core_request_rom_load(rompath);

	core_run();

	core_shutdown();

	return 0;
}