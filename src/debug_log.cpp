#include "debug_log.h"
#include <SDL3/SDL.h>
#include <iostream>
#include <cstdarg>
#include <string>

bool log_to_console = true;
bool log_to_file = true;
static FILE* logfile = NULL;

void debug_log(const char* fmt, ...) {
	va_list args;

	if (log_to_console) {
		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		va_end(args);
	}

	if (log_to_file) {
		if (!logfile) {
			// Anchor the log to the executable's folder. The working directory isn't reliable:
			// launching by dropping a ROM onto the executable can start us almost anywhere.
			const char* base_path = SDL_GetBasePath(); // Ends with a separator; owned by SDL, so don't free it
			const std::string log_dir = std::string(base_path ? base_path : "./") + "logs";
			SDL_CreateDirectory(log_dir.c_str()); // Also succeeds when the folder already exists
			const std::string log_path = log_dir + "/logfile.txt";

			logfile = fopen(log_path.c_str(), "w");
			if (!logfile) {
				printf("Failed to open %s for writing!\n", log_path.c_str());
				log_to_file = false;
				return;
			}
		}
		va_start(args, fmt);
		vfprintf(logfile, fmt, args);
		va_end(args);
	}
}

void debug_log_close_file() {
	if (logfile) {
		log_to_file = false;
		fclose(logfile);
		logfile = NULL;
	}
}