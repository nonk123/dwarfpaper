#pragma once

enum LogLevel {
	LOG_TRACE,
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR,
	LOG_FATAL,
	LOG_MAX,
};

__attribute__((noreturn)) void die();
void __log(const char*, enum LogLevel, const char*, int, ...);
#define _log(level, fmt, ...) __log(fmt, level, __FILE__, __LINE__, ##__VA_ARGS__)

#define trace(...) _log(LOG_TRACE, __VA_ARGS__)
#define debug(...) _log(LOG_DEBUG, __VA_ARGS__)
#define info(...) _log(LOG_INFO, __VA_ARGS__)
#define warn(...) _log(LOG_WARN, __VA_ARGS__)
#define error(...) _log(LOG_ERROR, __VA_ARGS__)
#define fatal(...) _log(LOG_FATAL, __VA_ARGS__), die();
#define expect(expr, msg, ...)                                                                                         \
	do {                                                                                                           \
		if (!(expr))                                                                                           \
			_log(LOG_FATAL, "Assertion " #expr " failed. " msg, ##__VA_ARGS__), die();                     \
	} while (0)
