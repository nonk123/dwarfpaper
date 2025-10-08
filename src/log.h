#pragma once

#include <stdio.h>

enum LogLevel {
	LOG_TRACE,
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR,
	LOG_FATAL,
};

extern enum LogLevel global_log_level;

#define LOG_OUT (stdout)
#define _log(level, linum, src_file, msg, ...)                                                                         \
	do {                                                                                                           \
		if ((level) < global_log_level)                                                                        \
			break;                                                                                         \
		fprintf(LOG_OUT, "%s: [%s:%d] -> " msg "\n", log_level_name((level)), file_basename((src_file)),       \
			(linum), ##__VA_ARGS__);                                                                       \
		fflush(LOG_OUT);                                                                                       \
	} while (0)
#define log(level, ...) _log(level, __LINE__, __FILE__, __VA_ARGS__)

#define trace(...) log(LOG_TRACE, __VA_ARGS__)
#define debug(...) log(LOG_DEBUG, __VA_ARGS__)
#define info(...) log(LOG_INFO, __VA_ARGS__)
#define warn(...) log(LOG_WARN, __VA_ARGS__)
#define error(...) log(LOG_ERROR, __VA_ARGS__)
#define fatal(...)                                                                                                     \
	do {                                                                                                           \
		log(LOG_FATAL, __VA_ARGS__);                                                                           \
		commit_seppuku();                                                                                      \
	} while (0)
#define expect(expr, ...)                                                                                              \
	do {                                                                                                           \
		if (!(expr))                                                                                           \
			fatal(__VA_ARGS__);                                                                            \
	} while (0)

const char* log_level_name(enum LogLevel);
const char* file_basename(const char*);
void commit_seppuku() __attribute__((noreturn));
