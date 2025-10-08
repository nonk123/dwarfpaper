#include <stdlib.h>

#include <SDL3/SDL_stdinc.h>

#include "log.h"

enum LogLevel global_log_level = LOG_INFO;

const char* log_level_name(enum LogLevel level) {
	switch (level) {
		case LOG_TRACE:
			return "TRACE";
		case LOG_DEBUG:
			return "DEBUG";
		case LOG_INFO:
			return "INFO";
		case LOG_WARN:
			return "WARN";
		case LOG_ERROR:
			return "ERROR";
		case LOG_FATAL:
			return "FATAL";
	}
	commit_seppuku();
}

const char* file_basename(const char* path) {
	const char* s = SDL_strrchr(path, '/');
	if (s == NULL)
		s = SDL_strrchr(path, '\\');
	return s == NULL ? path : s + 1;
}

void commit_seppuku() {
	exit(EXIT_FAILURE);
}
