#include <stdio.h> // really can't avoid these two:
#include <stdlib.h>

#include <SDL3/SDL_stdinc.h>

#include "log.h"

static enum LogLevel global_log_level = LOG_INFO;

static const char* file_basename(const char* path) {
	const char* s = SDL_strrchr(path, '/');
	if (s == NULL)
		s = SDL_strrchr(path, '\\');
	return s == NULL ? path : s + 1;
}

static const char* log_level_names[LOG_MAX] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
void __log(const char* fmt, enum LogLevel level, const char* file, int linum, ...) {
	if (level < global_log_level)
		return;

	static char buf[1024] = {0};
	int count = sprintf(buf, "%s: [%s:%d] -> ", log_level_names[level], file_basename(file), linum);
	va_list args;
	va_start(args, linum);
	vsprintf(buf + count, fmt, args);
	va_end(args);

	fprintf(stdout, "%s\n", buf);
	fflush(stdout);
}

void die() {
	exit(EXIT_FAILURE);
}
