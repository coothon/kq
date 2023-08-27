// Include like a normal header wherever needed.
// And to provide the implementation, put
// #define LIBCBASE_LOG_IMPLEMENTATION
// before including it in one translation unit.

#ifndef LIBCBASE_LOG_H_
#define LIBCBASE_LOG_H_

#include "common.h"

enum cb_log_levels {
	CB_LOG_LEVEL_FATAL = 0,
	CB_LOG_LEVEL_ERROR,
	CB_LOG_LEVEL_WARN,
	CB_LOG_LEVEL_INFO,
	CB_LOG_LEVEL_DEBUG,
	CB_LOG_LEVEL_TRACE,
	CB_LOG_LEVELS_COUNT,
};

// Log timestamps will be counted from when this function is called, so call it as early as possible.
extern void cb_log_init(uint initial_log_level);
extern void cb_log_level_set(uint log_level);

// For the module macros, define CB_LOG_MODULE at the top of any implementation file as a string name meaning whatever
// you want.

__attribute__((format(printf, 5, 6))) extern void cb_log_internal_verbose(const char func_name[restrict static 1],
                                                                          const char file_name[restrict static 1],
                                                                          uint       line_num,
                                                                          uint       level,
                                                                          const char fmt[restrict static 1],
                                                                          ...);
__attribute__((format(printf, 2, 3))) extern void cb_log_internal_quiet(uint       level,
                                                                        const char fmt[restrict static 1],
                                                                        ...);

__attribute__((format(printf, 6, 7))) extern void cb_log_internal_verbose_module(const char
                                                                                         func_name[restrict static 1],
                                                                                 const char
                                                                                         file_name[restrict static 1],
                                                                                 uint    line_num,
                                                                                 uint    level,
                                                                                 const char module[restrict static 1],
                                                                                 const char fmt[restrict static 1],
                                                                                 ...);
__attribute__((format(printf, 3, 4))) extern void cb_log_internal_quiet_module(uint       level,
                                                                               const char module[restrict static 1],
                                                                               const char fmt[restrict static 1],
                                                                               ...);

__attribute__((format(printf, 6, 7))) extern void cb_log_internal_verbose_errno(const char func_name[restrict static 1],
                                                                                const char file_name[restrict static 1],
                                                                                uint       line_num,
                                                                                uint       level,
                                                                                int        errnum,
                                                                                const char fmt[restrict static 1],
                                                                                ...);
__attribute__((format(printf, 3, 4))) extern void cb_log_internal_quiet_errno(uint       level,
                                                                              int        errnum,
                                                                              const char fmt[restrict static 1],
                                                                              ...);

#if CB_DEBUG
#	define LOG_FATAL(...) cb_log_internal_verbose(__func__, __FILE__, __LINE__, CB_LOG_LEVEL_FATAL, __VA_ARGS__)
#	define LOG_ERROR(...) cb_log_internal_verbose(__func__, __FILE__, __LINE__, CB_LOG_LEVEL_ERROR, __VA_ARGS__)
#	define LOG_WARN(...)  cb_log_internal_verbose(__func__, __FILE__, __LINE__, CB_LOG_LEVEL_WARN, __VA_ARGS__)
#	define LOG_INFO(...)  cb_log_internal_verbose(__func__, __FILE__, __LINE__, CB_LOG_LEVEL_INFO, __VA_ARGS__)
#	define LOG_DEBUG(...) cb_log_internal_verbose(__func__, __FILE__, __LINE__, CB_LOG_LEVEL_DEBUG, __VA_ARGS__)
#	define LOG_TRACE(...) cb_log_internal_verbose(__func__, __FILE__, __LINE__, CB_LOG_LEVEL_TRACE, __VA_ARGS__)

#	define LOG_MFATAL(...)                                    \
		cb_log_internal_verbose_module(__func__,           \
		                               __FILE__,           \
		                               __LINE__,           \
		                               CB_LOG_LEVEL_FATAL, \
		                               CB_LOG_MODULE,      \
		                               __VA_ARGS__)
#	define LOG_MERROR(...)                                    \
		cb_log_internal_verbose_module(__func__,           \
		                               __FILE__,           \
		                               __LINE__,           \
		                               CB_LOG_LEVEL_ERROR, \
		                               CB_LOG_MODULE,      \
		                               __VA_ARGS__)
#	define LOG_MWARN(...)                                    \
		cb_log_internal_verbose_module(__func__,          \
		                               __FILE__,          \
		                               __LINE__,          \
		                               CB_LOG_LEVEL_WARN, \
		                               CB_LOG_MODULE,     \
		                               __VA_ARGS__)
#	define LOG_MINFO(...)                                    \
		cb_log_internal_verbose_module(__func__,          \
		                               __FILE__,          \
		                               __LINE__,          \
		                               CB_LOG_LEVEL_INFO, \
		                               CB_LOG_MODULE,     \
		                               __VA_ARGS__)
#	define LOG_MDEBUG(...)                                    \
		cb_log_internal_verbose_module(__func__,           \
		                               __FILE__,           \
		                               __LINE__,           \
		                               CB_LOG_LEVEL_DEBUG, \
		                               CB_LOG_MODULE,      \
		                               __VA_ARGS__)
#	define LOG_MTRACE(...)                                    \
		cb_log_internal_verbose_module(__func__,           \
		                               __FILE__,           \
		                               __LINE__,           \
		                               CB_LOG_LEVEL_TRACE, \
		                               CB_LOG_MODULE,      \
		                               __VA_ARGS__)

#	define LOG_EFATAL(errnum, ...) \
		cb_log_internal_verbose_errno(__func__, __FILE__, __LINE__, CB_LOG_LEVEL_FATAL, errnum, __VA_ARGS__)
#	define LOG_EERROR(errnum, ...) \
		cb_log_internal_verbose_errno(__func__, __FILE__, __LINE__, CB_LOG_LEVEL_ERROR, errnum, __VA_ARGS__)
#	define LOG_EWARN(errnum, ...) \
		cb_log_internal_verbose_errno(__func__, __FILE__, __LINE__, CB_LOG_LEVEL_WARN, errnum, __VA_ARGS__)
#	define LOG_EDEBUG(errnum, ...) \
		cb_log_internal_verbose_errno(__func__, __FILE__, __LINE__, CB_LOG_LEVEL_DEBUG, errnum, __VA_ARGS__)
#else
#	define LOG_FATAL(...) cb_log_internal_quiet(CB_LOG_LEVEL_FATAL, __VA_ARGS__)
#	define LOG_ERROR(...) cb_log_internal_quiet(CB_LOG_LEVEL_ERROR, __VA_ARGS__)
#	define LOG_WARN(...)  cb_log_internal_quiet(CB_LOG_LEVEL_WARN, __VA_ARGS__)
#	define LOG_INFO(...)  cb_log_internal_quiet(CB_LOG_LEVEL_INFO, __VA_ARGS__)
#	define LOG_DEBUG(...) cb_log_internal_quiet(CB_LOG_LEVEL_DEBUG, __VA_ARGS__)
#	define LOG_TRACE(...) cb_log_internal_quiet(CB_LOG_LEVEL_TRACE, __VA_ARGS__)

#	define LOG_MFATAL(...) cb_log_internal_quiet_module(CB_LOG_LEVEL_FATAL, CB_LOG_MODULE, __VA_ARGS__)
#	define LOG_MERROR(...) cb_log_internal_quiet_module(CB_LOG_LEVEL_ERROR, CB_LOG_MODULE, __VA_ARGS__)
#	define LOG_MWARN(...)  cb_log_internal_quiet_module(CB_LOG_LEVEL_WARN, CB_LOG_MODULE, __VA_ARGS__)
#	define LOG_MINFO(...)  cb_log_internal_quiet_module(CB_LOG_LEVEL_INFO, CB_LOG_MODULE, __VA_ARGS__)
#	define LOG_MDEBUG(...) cb_log_internal_quiet_module(CB_LOG_LEVEL_DEBUG, CB_LOG_MODULE, __VA_ARGS__)
#	define LOG_MTRACE(...) cb_log_internal_quiet_module(CB_LOG_LEVEL_TRACE, CB_LOG_MODULE, __VA_ARGS__)

#	define LOG_EFATAL(errnum, ...) cb_log_internal_quiet_errno(CB_LOG_LEVEL_FATAL, errnum, __VA_ARGS__)
#	define LOG_EERROR(errnum, ...) cb_log_internal_quiet_errno(CB_LOG_LEVEL_ERROR, errnum, __VA_ARGS__)
#	define LOG_EWARN(errnum, ...)  cb_log_internal_quiet_errno(CB_LOG_LEVEL_WARN, errnum, __VA_ARGS__)
#	define LOG_EDEBUG(errnum, ...) cb_log_internal_quiet_errno(CB_LOG_LEVEL_DEBUG, errnum, __VA_ARGS__)
#endif /* CB_DEBUG */

#endif /* LIBCBASE_LOG_H_ */


#define LIBCBASE_LOG_IMPLEMENTATION
#ifdef LIBCBASE_LOG_IMPLEMENTATION
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// Prints everything below, including its own level.
static uint cb_log_level;

// Set this with clock_gettime(CLOCK_REALTIME, &time_at_start) before any logs
// for the timestamps to work properly.
static struct timespec cb_log_time_at_start;

static const char *const log_color[CB_LOG_LEVELS_COUNT] = {
	[CB_LOG_LEVEL_FATAL] = "\033[1;31m",
	[CB_LOG_LEVEL_ERROR] = "\033[31m",
	[CB_LOG_LEVEL_WARN]  = "\033[34m",
	[CB_LOG_LEVEL_INFO]  = "\033[33m",
	[CB_LOG_LEVEL_DEBUG] = "\033[1;32m",
	[CB_LOG_LEVEL_TRACE] = "\033[90m",
};

static const char *const log_banner[CB_LOG_LEVELS_COUNT] = {
	[CB_LOG_LEVEL_FATAL] = "Fatal error:",
	[CB_LOG_LEVEL_ERROR] = "Error:",
	[CB_LOG_LEVEL_WARN]  = "Warning:",
	[CB_LOG_LEVEL_INFO]  = "Info:",
	[CB_LOG_LEVEL_DEBUG] = "Debug:",
	[CB_LOG_LEVEL_TRACE] = "Trace:",
};

static __always_inline void timespec_subtract(struct timespec       left[restrict static 1],
                                              const struct timespec right[restrict static 1]);

void cb_log_init(uint initial_log_level) {
	clock_gettime(CLOCK_REALTIME, &cb_log_time_at_start);
	cb_log_level = initial_log_level;
}

void cb_log_level_set(uint log_level) {
	cb_log_level = log_level;
}

__attribute__((format(printf, 5, 6))) void cb_log_internal_verbose(const char func_name[restrict static 1],
                                                                   const char file_name[restrict static 1],
                                                                   uint       line_num,
                                                                   uint       level,
                                                                   const char fmt[restrict static 1],
                                                                   ...) {
	if (level > cb_log_level)
		return;

	va_list         args;
	struct timespec log_time;
	clock_gettime(CLOCK_REALTIME, &log_time);
	timespec_subtract(&log_time, &cb_log_time_at_start);

	// Lock file to ensure the entire message gets outputted in one piece.
	// I'm guessing it's a recursive mutex.
	flockfile(stderr);
	fprintf(stderr,
	        "%s[%ld.%.3lu] [%s:%s():%u] %s\033[0m ",
	        log_color[level],
	        log_time.tv_sec,
	        (ulong)log_time.tv_nsec / 1000000UL,
	        file_name,
	        func_name,
	        line_num,
	        log_banner[level]);

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	fputs("\n", stderr);
	funlockfile(stderr);
}

__attribute__((format(printf, 2, 3))) void cb_log_internal_quiet(uint level, const char fmt[restrict static 1], ...) {
	if (level > cb_log_level)
		return;

	va_list args;

	flockfile(stderr);
	fprintf(stderr, "%s%s\033[0m ", log_color[level], log_banner[level]);

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	fputs("\n", stderr);
	funlockfile(stderr);
}

__attribute__((format(printf, 6, 7))) extern void cb_log_internal_verbose_module(const char
                                                                                         func_name[restrict static 1],
                                                                                 const char
                                                                                         file_name[restrict static 1],
                                                                                 uint    line_num,
                                                                                 uint    level,
                                                                                 const char module[restrict static 1],
                                                                                 const char fmt[restrict static 1],
                                                                                 ...) {
	if (level > cb_log_level)
		return;

	va_list         args;
	struct timespec log_time;
	clock_gettime(CLOCK_REALTIME, &log_time);
	timespec_subtract(&log_time, &cb_log_time_at_start);

	flockfile(stderr);
	fprintf(stderr,
	        "%s[%ld.%.3lu] [MODULE %s] [%s:%s():%u] %s\033[0m ",
	        log_color[level],
	        log_time.tv_sec,
	        (ulong)log_time.tv_nsec / 1000000UL,
	        module,
	        file_name,
	        func_name,
	        line_num,
	        log_banner[level]);

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	fputs("\n", stderr);
	funlockfile(stderr);
}

__attribute__((format(printf, 3, 4))) extern void cb_log_internal_quiet_module(uint       level,
                                                                               const char module[restrict static 1],
                                                                               const char fmt[restrict static 1],
                                                                               ...) {
	if (level > cb_log_level)
		return;

	va_list args;

	flockfile(stderr);
	fprintf(stderr, "%s[MODULE %s] %s\033[0m ", log_color[level], module, log_banner[level]);

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	fputs("\n", stderr);
	funlockfile(stderr);
}

__attribute__((format(printf, 6, 7))) void cb_log_internal_verbose_errno(const char func_name[restrict static 1],
                                                                         const char file_name[restrict static 1],
                                                                         uint       line_num,
                                                                         uint       level,
                                                                         int        errnum,
                                                                         const char fmt[restrict static 1],
                                                                         ...) {
	if (level > cb_log_level)
		return;

	va_list         args;
	struct timespec log_time;
	clock_gettime(CLOCK_REALTIME, &log_time);
	timespec_subtract(&log_time, &cb_log_time_at_start);

	flockfile(stderr);
	fprintf(stderr,
	        "%s[%ld.%.3lu] [%s:%s():%u] [%s(%d) %s] %s\033[0m ",
	        log_color[level],
	        log_time.tv_sec,
	        (ulong)log_time.tv_nsec / 1000000UL,
	        file_name,
	        func_name,
	        line_num,
	        strerrorname_np(errnum),
	        errnum,
	        strerror(errnum),
	        log_banner[level]);

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	fputs("\n", stderr);
	funlockfile(stderr);
}

__attribute__((format(printf, 3, 4))) void cb_log_internal_quiet_errno(uint       level,
                                                                       int        errnum,
                                                                       const char fmt[restrict static 1],
                                                                       ...) {
	if (level > cb_log_level)
		return;

	va_list args;

	flockfile(stderr);
	fprintf(stderr,
	        "%s[%s(%d) %s] %s\033[0m ",
	        log_color[level],
	        strerrorname_np(errnum),
	        errnum,
	        strerror(errnum),
	        log_banner[level]);

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	fputs("\n", stderr);
	funlockfile(stderr);
}

static __always_inline void timespec_subtract(struct timespec       left[restrict static 1],
                                              const struct timespec right[restrict static 1]) {
	left->tv_nsec   -= right->tv_nsec;
	long underflowed = left->tv_nsec < 0L;
	left->tv_sec    -= right->tv_sec + underflowed;
	left->tv_nsec   += underflowed * 1000000000L;
}

#undef LIBCBASE_LOG_IMPLEMENTATION
#endif /* LIBCBASE_LOG_IMPLEMENTATION */
