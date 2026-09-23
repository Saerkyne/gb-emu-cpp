#include "debug_log.h"
#include <cstdarg>
#include <iostream>

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
			const errno_t err = fopen_s(&logfile, "./logs/logfile.txt", "w");
			if (err != 0) {
				printf("Failed to open logfile.txt for writing!\n");
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