#include <cstdio>
#include <cstring>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "emulator_core.h"

int main(int argc, char* argv[]) 
{
	// Test Mode Block
	const char* rompath = NULL;

	for (int i = 1; i < argc; ++i)
	{
		const char* arg = argv[i];

		if (strcmp(arg, "-t") == 0 || strcmp(arg, "--test-mode") == 0)
		{
			test_mode = true;
		}
		else if (strcmp(arg, "-r") == 0 || strcmp(arg, "--rom") == 0)
		{
			if (i + 1 >= argc)
			{
				printf("%s requires a path argument.\n", arg);
				return CORE_EXIT_BAD_ARGS;
			}
			rompath = argv[++i]; // move to the next argument before using it
		}
		else if (arg[0] == '-')
		{
			printf("Unknown option: %s\n", arg);
			return CORE_EXIT_BAD_ARGS;
		}
		else
		{
			rompath = arg; // a bare path: droppped on the exe, or using "open with" option
		}
	}

	if (test_mode)
	{
		core_exit_code = CORE_EXIT_TEST_NO_RESULT; // Stays here unless a test replaces it
	}
	// End Test Mode Block

	int error = core_init();
	if (error != 0)
	{
		return error;
	}

	core_request_rom_load(rompath);

	core_run();

	core_shutdown();

	return core_exit_code;
}