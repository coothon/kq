// Include like a normal header wherever needed.
// And to provide the implementation, put
// #define LIBCBASE_LOG_IMPLEMENTATION
// before including it in one translation unit.

#ifndef LIBCBASE_LOG_H_
#define LIBCBASE_LOG_H_

#include <stdbool.h>
#include <stdio.h>

#include "common.h"

#define CB_LOG_LEVEL_FATAL  (0U)
#define CB_LOG_LEVEL_ERROR  (CB_LOG_LEVEL_FATAL + 1U)
#define CB_LOG_LEVEL_WARN   (CB_LOG_LEVEL_ERROR + 1U)
#define CB_LOG_LEVEL_INFO   (CB_LOG_LEVEL_WARN + 1U)
#define CB_LOG_LEVEL_DEBUG  (CB_LOG_LEVEL_INFO + 1U)
#define CB_LOG_LEVEL_TRACE  (CB_LOG_LEVEL_DEBUG + 1U)
#define CB_LOG_LEVELS_COUNT (CB_LOG_LEVEL_TRACE + 1U)

// Define this to control compile-time removal of log calls. Less overhead, I suppose.
#ifndef CB_LOG_LEVEL_COMPILE_TIME_MIN
#define CB_LOG_LEVEL_COMPILE_TIME_MIN CB_LOG_LEVEL_TRACE
#endif

#define CB_LOG_FEATURE_TIME     (1U)
#define CB_LOG_FEATURE_LOCATION (1U << 1U)
#define CB_LOG_FEATURE_MODULE   (1U << 2U)
#define CB_LOG_FEATURE_ERRNO    (1U << 3U)

#define CB_ATTR_PRINTF(a, b) __attribute__((format(printf, (a), (b))))

// Log timestamps will be counted from when this function is called, so call it as early as possible.
extern void cb_log_init(FILE f[static 1], uint initial_log_level, bool reltime, bool use_colours);
extern void cb_log_set_level(uint log_level);
extern void cb_log_set_reltime(bool yesno);
extern void cb_log_set_use_colours(bool yesno);
extern void cb_log_infer_use_colours(void);

// For the module macros, define CB_LOG_MODULE at the top of any implementation file as a string name meaning whatever
// you want.

CB_ATTR_PRINTF(8, 9)
extern void cb_log_internal(uint level,
                            uint features,
                            const char *restrict func_name,
                            const char *restrict file_name,
                            uint line_num,
                            const char *restrict module,
                            int        errnum,
                            const char fmt[restrict static 1],
                            ...);


// Implements compile-time log level exclusion.
#if CB_LOG_LEVEL_COMPILE_TIME_MIN >= CB_LOG_LEVEL_FATAL
#define CB_LOG_TEMPLATE_FATAL(...) cb_log_internal(CB_LOG_LEVEL_FATAL, __VA_ARGS__)
#else
#define CB_LOG_TEMPLATE_FATAL(...)
#endif

#if CB_LOG_LEVEL_COMPILE_TIME_MIN >= CB_LOG_LEVEL_ERROR
#define CB_LOG_TEMPLATE_ERROR(...) cb_log_internal(CB_LOG_LEVEL_ERROR, __VA_ARGS__)
#else
#define CB_LOG_TEMPLATE_ERROR(...)
#endif

#if CB_LOG_LEVEL_COMPILE_TIME_MIN >= CB_LOG_LEVEL_WARN
#define CB_LOG_TEMPLATE_WARN(...) cb_log_internal(CB_LOG_LEVEL_WARN, __VA_ARGS__)
#else
#define CB_LOG_TEMPLATE_WARN(...)
#endif

#if CB_LOG_LEVEL_COMPILE_TIME_MIN >= CB_LOG_LEVEL_INFO
#define CB_LOG_TEMPLATE_INFO(...) cb_log_internal(CB_LOG_LEVEL_INFO, __VA_ARGS__)
#else
#define CB_LOG_TEMPLATE_INFO(...)
#endif

#if CB_LOG_LEVEL_COMPILE_TIME_MIN >= CB_LOG_LEVEL_DEBUG
#define CB_LOG_TEMPLATE_DEBUG(...) cb_log_internal(CB_LOG_LEVEL_DEBUG, __VA_ARGS__)
#else
#define CB_LOG_TEMPLATE_DEBUG(...)
#endif

#if CB_LOG_LEVEL_COMPILE_TIME_MIN >= CB_LOG_LEVEL_TRACE
#define CB_LOG_TEMPLATE_TRACE(...) cb_log_internal(CB_LOG_LEVEL_TRACE, __VA_ARGS__)
#else
#define CB_LOG_TEMPLATE_TRACE(...)
#endif

// clang-format off
#if CB_DEBUG
// Normal logging functions.
#define LOG_FATAL(...) CB_LOG_TEMPLATE_FATAL(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_LOCATION, __func__, __FILE__, __LINE__, 0, 0, __VA_ARGS__)
#define LOG_ERROR(...) CB_LOG_TEMPLATE_ERROR(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_LOCATION, __func__, __FILE__, __LINE__, 0, 0, __VA_ARGS__)
#define LOG_WARN(...) CB_LOG_TEMPLATE_WARN(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_LOCATION, __func__, __FILE__, __LINE__, 0, 0, __VA_ARGS__)
#define LOG_INFO(...) CB_LOG_TEMPLATE_INFO(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_LOCATION, __func__, __FILE__, __LINE__, 0, 0, __VA_ARGS__)
#define LOG_DEBUG(...) CB_LOG_TEMPLATE_DEBUG(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_LOCATION, __func__, __FILE__, __LINE__, 0, 0, __VA_ARGS__)
#define LOG_TRACE(...) CB_LOG_TEMPLATE_TRACE(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_LOCATION, __func__, __FILE__, __LINE__, 0, 0, __VA_ARGS__)

// Functions that require CB_LOG_MODULE to be defined, and print it.
#define LOGM_FATAL(...) CB_LOG_TEMPLATE_FATAL(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_LOCATION | CB_LOG_FEATURE_MODULE, __func__, __FILE__, __LINE__, CB_LOG_MODULE, 0, __VA_ARGS__)
#define LOGM_ERROR(...) CB_LOG_TEMPLATE_ERROR(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_LOCATION | CB_LOG_FEATURE_MODULE, __func__, __FILE__, __LINE__, CB_LOG_MODULE, 0, __VA_ARGS__)
#define LOGM_WARN(...) CB_LOG_TEMPLATE_WARN(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_LOCATION | CB_LOG_FEATURE_MODULE, __func__, __FILE__, __LINE__, CB_LOG_MODULE, 0, __VA_ARGS__)
#define LOGM_INFO(...) CB_LOG_TEMPLATE_INFO(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_LOCATION | CB_LOG_FEATURE_MODULE, __func__, __FILE__, __LINE__, CB_LOG_MODULE, 0, __VA_ARGS__)
#define LOGM_DEBUG(...) CB_LOG_TEMPLATE_DEBUG(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_LOCATION | CB_LOG_FEATURE_MODULE, __func__, __FILE__, __LINE__, CB_LOG_MODULE, 0, __VA_ARGS__)
#define LOGM_TRACE(...) CB_LOG_TEMPLATE_TRACE(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_LOCATION | CB_LOG_FEATURE_MODULE, __func__, __FILE__, __LINE__, CB_LOG_MODULE, 0, __VA_ARGS__)

// Functions that require an errnum argument that represents an errno value.
#define LOGE_FATAL(errnum, ...) CB_LOG_TEMPLATE_FATAL(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_LOCATION | CB_LOG_FEATURE_ERRNO, __func__, __FILE__, __LINE__, 0, (errnum), __VA_ARGS__)
#define LOGE_ERROR(errnum, ...) CB_LOG_TEMPLATE_ERROR(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_LOCATION | CB_LOG_FEATURE_ERRNO, __func__, __FILE__, __LINE__, 0, (errnum), __VA_ARGS__)
#define LOGE_WARN(errnum, ...) CB_LOG_TEMPLATE_WARN(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_LOCATION | CB_LOG_FEATURE_ERRNO, __func__, __FILE__, __LINE__, 0, (errnum), __VA_ARGS__)
#define LOGE_INFO(errnum, ...) CB_LOG_TEMPLATE_INFO(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_LOCATION | CB_LOG_FEATURE_ERRNO, __func__, __FILE__, __LINE__, 0, (errnum), __VA_ARGS__)
#define LOGE_DEBUG(errnum, ...) CB_LOG_TEMPLATE_DEBUG(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_LOCATION | CB_LOG_FEATURE_ERRNO, __func__, __FILE__, __LINE__, 0, (errnum), __VA_ARGS__)
#define LOGE_TRACE(errnum, ...) CB_LOG_TEMPLATE_TRACE(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_LOCATION | CB_LOG_FEATURE_ERRNO, __func__, __FILE__, __LINE__, 0, (errnum), __VA_ARGS__)

// Functions that require both CB_LOG_MODULE and an errnum.
#define LOGEM_FATAL(errnum, ...) CB_LOG_TEMPLATE_FATAL(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_LOCATION | CB_LOG_FEATURE_MODULE | CB_LOG_FEATURE_ERRNO, __func__, __FILE__, __LINE__, CB_LOG_MODULE, (errnum), __VA_ARGS__)
#define LOGEM_ERROR(errnum, ...) CB_LOG_TEMPLATE_ERROR(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_LOCATION | CB_LOG_FEATURE_MODULE | CB_LOG_FEATURE_ERRNO, __func__, __FILE__, __LINE__, CB_LOG_MODULE, (errnum), __VA_ARGS__)
#define LOGEM_WARN(errnum, ...) CB_LOG_TEMPLATE_WARN(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_LOCATION | CB_LOG_FEATURE_MODULE | CB_LOG_FEATURE_ERRNO, __func__, __FILE__, __LINE__, CB_LOG_MODULE, (errnum), __VA_ARGS__)
#define LOGEM_INFO(errnum, ...) CB_LOG_TEMPLATE_INFO(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_LOCATION | CB_LOG_FEATURE_MODULE | CB_LOG_FEATURE_ERRNO, __func__, __FILE__, __LINE__, CB_LOG_MODULE, (errnum), __VA_ARGS__)
#define LOGEM_DEBUG(errnum, ...) CB_LOG_TEMPLATE_DEBUG(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_LOCATION | CB_LOG_FEATURE_MODULE | CB_LOG_FEATURE_ERRNO, __func__, __FILE__, __LINE__, CB_LOG_MODULE, (errnum), __VA_ARGS__)
#define LOGEM_TRACE(errnum, ...) CB_LOG_TEMPLATE_TRACE(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_LOCATION | CB_LOG_FEATURE_MODULE | CB_LOG_FEATURE_ERRNO, __func__, __FILE__, __LINE__, CB_LOG_MODULE, (errnum), __VA_ARGS__)

#else

// More silent versions of the above; for non-debug builds.
#define LOG_FATAL(...) CB_LOG_TEMPLATE_FATAL(CB_LOG_FEATURE_TIME, 0, 0, 0, 0, 0, __VA_ARGS__)
#define LOG_ERROR(...) CB_LOG_TEMPLATE_ERROR(CB_LOG_FEATURE_TIME, 0, 0, 0, 0, 0, __VA_ARGS__)
#define LOG_WARN(...) CB_LOG_TEMPLATE_WARN(CB_LOG_FEATURE_TIME, 0, 0, 0, 0, 0, __VA_ARGS__)
#define LOG_INFO(...) CB_LOG_TEMPLATE_INFO(CB_LOG_FEATURE_TIME, 0, 0, 0, 0, 0, __VA_ARGS__)
#define LOG_DEBUG(...) CB_LOG_TEMPLATE_DEBUG(CB_LOG_FEATURE_TIME, 0, 0, 0, 0, 0, __VA_ARGS__)
#define LOG_TRACE(...) CB_LOG_TEMPLATE_TRACE(CB_LOG_FEATURE_TIME, 0, 0, 0, 0, 0, __VA_ARGS__)

#define LOGM_FATAL(...) CB_LOG_TEMPLATE_FATAL(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_MODULE, 0, 0, 0, CB_LOG_MODULE, 0, __VA_ARGS__)
#define LOGM_ERROR(...) CB_LOG_TEMPLATE_ERROR(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_MODULE, 0, 0, 0, CB_LOG_MODULE, 0, __VA_ARGS__)
#define LOGM_WARN(...) CB_LOG_TEMPLATE_WARN(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_MODULE, 0, 0, 0, CB_LOG_MODULE, 0, __VA_ARGS__)
#define LOGM_INFO(...) CB_LOG_TEMPLATE_INFO(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_MODULE, 0, 0, 0, CB_LOG_MODULE, 0, __VA_ARGS__)
#define LOGM_DEBUG(...) CB_LOG_TEMPLATE_DEBUG(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_MODULE, 0, 0, 0, CB_LOG_MODULE, 0, __VA_ARGS__)
#define LOGM_TRACE(...) CB_LOG_TEMPLATE_TRACE(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_MODULE, 0, 0, 0, CB_LOG_MODULE, 0, __VA_ARGS__)

#define LOGE_FATAL(errnum, ...) CB_LOG_TEMPLATE_FATAL(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_ERRNO, 0, 0, 0, 0, (errnum), __VA_ARGS__)
#define LOGE_ERROR(errnum, ...) CB_LOG_TEMPLATE_ERROR(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_ERRNO, 0, 0, 0, 0, (errnum), __VA_ARGS__)
#define LOGE_WARN(errnum, ...) CB_LOG_TEMPLATE_WARN(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_ERRNO, 0, 0, 0, 0, (errnum), __VA_ARGS__)
#define LOGE_INFO(errnum, ...) CB_LOG_TEMPLATE_INFO(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_ERRNO, 0, 0, 0, 0, (errnum), __VA_ARGS__)
#define LOGE_DEBUG(errnum, ...) CB_LOG_TEMPLATE_DEBUG(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_ERRNO, 0, 0, 0, 0, (errnum), __VA_ARGS__)
#define LOGE_TRACE(errnum, ...) CB_LOG_TEMPLATE_TRACE(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_ERRNO, 0, 0, 0, 0, (errnum), __VA_ARGS__)

#define LOGEM_FATAL(errnum, ...) CB_LOG_TEMPLATE_FATAL(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_MODULE | CB_LOG_FEATURE_ERRNO, 0, 0, 0, CB_LOG_MODULE, (errnum), __VA_ARGS__)
#define LOGEM_ERROR(errnum, ...) CB_LOG_TEMPLATE_ERROR(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_MODULE | CB_LOG_FEATURE_ERRNO, 0, 0, 0, CB_LOG_MODULE, (errnum), __VA_ARGS__)
#define LOGEM_WARN(errnum, ...) CB_LOG_TEMPLATE_WARN(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_MODULE | CB_LOG_FEATURE_ERRNO, 0, 0, 0, CB_LOG_MODULE, (errnum), __VA_ARGS__)
#define LOGEM_INFO(errnum, ...) CB_LOG_TEMPLATE_INFO(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_MODULE | CB_LOG_FEATURE_ERRNO, 0, 0, 0, CB_LOG_MODULE, (errnum), __VA_ARGS__)
#define LOGEM_DEBUG(errnum, ...) CB_LOG_TEMPLATE_DEBUG(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_MODULE | CB_LOG_FEATURE_ERRNO, 0, 0, 0, CB_LOG_MODULE, (errnum), __VA_ARGS__)
#define LOGEM_TRACE(errnum, ...) CB_LOG_TEMPLATE_TRACE(CB_LOG_FEATURE_TIME | CB_LOG_FEATURE_MODULE | CB_LOG_FEATURE_ERRNO, 0, 0, 0, CB_LOG_MODULE, (errnum), __VA_ARGS__)
#endif /* CB_DEBUG */
// clang-format on

#endif /* LIBCBASE_LOG_H_ */



#ifdef LIBCBASE_LOG_IMPLEMENTATION
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static void cb_log_print_time(void);
static void cb_log_print_location(const char func_name[restrict static 1], const char file_name[restrict static 1], uint line_num);
static void cb_log_print_module(const char module[static 1]);
static void cb_log_print_errno(int errnum);

// Prints everything below, including its own level.
static uint cb_log_level = CB_LOG_LEVEL_TRACE;

// Controls whether or not to output colour.
static bool cb_log_use_colours = false;
// Controls whether to use absolute or relative logging time.
static bool cb_log_reltime = false;

// Set this with clock_gettime(CLOCK_REALTIME, &time_at_start) before any logs
// for the timestamps to work properly.
static struct timespec cb_log_time_at_start = {0};
static struct timespec cb_log_previous_time = {0};

// FILE to output to.
static FILE *cb_log_file = 0;

static const char *const log_colour[CB_LOG_LEVELS_COUNT] = {
	[CB_LOG_LEVEL_FATAL] = "\033[1;31m", [CB_LOG_LEVEL_ERROR] = "\033[31m",   [CB_LOG_LEVEL_WARN] = "\033[34m",
	[CB_LOG_LEVEL_INFO] = "\033[33m",    [CB_LOG_LEVEL_DEBUG] = "\033[1;32m", [CB_LOG_LEVEL_TRACE] = "\033[90m",
};

static const char *const log_banner[CB_LOG_LEVELS_COUNT] = {
	[CB_LOG_LEVEL_FATAL] = "Fatal error:", [CB_LOG_LEVEL_ERROR] = "Error:", [CB_LOG_LEVEL_WARN] = "Warning:",
	[CB_LOG_LEVEL_INFO] = "Info:",         [CB_LOG_LEVEL_DEBUG] = "Debug:", [CB_LOG_LEVEL_TRACE] = "Trace:",
};

static __always_inline void timespec_subtract(struct timespec left[restrict static 1], const struct timespec right[restrict static 1]);

void cb_log_init(FILE f[static 1], uint initial_log_level, bool reltime, bool use_colours) {
	cb_log_file        = f;
	cb_log_level       = initial_log_level;
	cb_log_use_colours = use_colours;
	cb_log_reltime     = reltime;
	clock_gettime(CLOCK_REALTIME, &cb_log_time_at_start);
	cb_log_previous_time = cb_log_time_at_start;
}

void cb_log_set_level(uint log_level) {
	cb_log_level = log_level;
}

void cb_log_set_reltime(bool yesno) {
	cb_log_reltime = yesno;
}

void cb_log_set_use_colours(bool yesno) {
	cb_log_use_colours = yesno;
}

void cb_log_infer_use_colours(void) {
	cb_log_use_colours = isatty(fileno(cb_log_file));
}

CB_ATTR_PRINTF(8, 9)
extern void cb_log_internal(uint level,
                            uint features,
                            const char *restrict func_name,
                            const char *restrict file_name,
                            uint line_num,
                            const char *restrict module,
                            int        errnum,
                            const char fmt[restrict static 1],
                            ...) {
	if (level > cb_log_level)
		return;

	flockfile(cb_log_file);
	if (cb_log_use_colours)
		fprintf(cb_log_file, "%s", log_colour[level]);
	if (features & CB_LOG_FEATURE_TIME)
		cb_log_print_time();
	if (features & CB_LOG_FEATURE_LOCATION)
		cb_log_print_location(func_name, file_name, line_num);
	if (features & CB_LOG_FEATURE_MODULE)
		cb_log_print_module(module);
	if (features & CB_LOG_FEATURE_ERRNO)
		cb_log_print_errno(errnum);
	fprintf(cb_log_file, cb_log_use_colours ? "%s\033[0m " : "%s ", log_banner[level]);

	// User-supplied formatting.
	va_list args;
	va_start(args, fmt);
	vfprintf(cb_log_file, fmt, args);
	va_end(args);

	fputs("\n", cb_log_file);
	funlockfile(cb_log_file);

	fflush(cb_log_file);
}

// They each print a space after.
static void cb_log_print_time(void) {
	struct timespec log_time, tmp;
	clock_gettime(CLOCK_REALTIME, &log_time);
	tmp = log_time;
	timespec_subtract(&log_time, cb_log_reltime ? &cb_log_previous_time : &cb_log_time_at_start);
	cb_log_previous_time = tmp;

	// Break down log_time; print parts as necessary.
	register const ulong ms    = (ulong)log_time.tv_nsec / 1000000UL;
	register const ulong d_sec = (ulong)log_time.tv_sec;
	if (d_sec < 60UL) {
		fprintf(cb_log_file, cb_log_reltime ? "[Δ%lu.%.3lus] " : "[+%lu.%.3lus] ", d_sec, ms);
		return;
	}

	register const ulong sec   = d_sec % 60UL;
	register const ulong d_min = d_sec / 60UL;
	if (d_min < 60UL) {
		fprintf(cb_log_file, cb_log_reltime ? "[Δ%lum %lu.%.3lus] " : "[+%lum %lu.%.3lus] ", d_min, sec, ms);
		return;
	}

	register const ulong min  = d_min % 60UL;
	register const ulong hour = d_min / 60UL;

	fprintf(cb_log_file, cb_log_reltime ? "[Δ%luh %lum %lu.%.3lus] " : "[+%luh %lum %lu.%.3lus] ", hour, min, sec, ms);
}

static void cb_log_print_location(const char func_name[restrict static 1], const char file_name[restrict static 1], uint line_num) {
	fprintf(cb_log_file, "[%s:%u:%s()] ", file_name, line_num, func_name);
}

static void cb_log_print_module(const char module[static 1]) {
	fprintf(cb_log_file, "[MODULE: %s] ", module);
}

static void cb_log_print_errno(int errnum) {
	fprintf(cb_log_file, "[%s(%d): %s.] ", strerrorname_np(errnum), errnum, strerror(errnum));
}

static __always_inline void timespec_subtract(struct timespec left[restrict static 1], const struct timespec right[restrict static 1]) {
	left->tv_nsec   -= right->tv_nsec;
	long underflowed = left->tv_nsec < 0L;
	left->tv_sec    -= right->tv_sec + underflowed;
	left->tv_nsec   += underflowed * 1000000000L;
}

#undef LIBCBASE_LOG_IMPLEMENTATION
#endif /* LIBCBASE_LOG_IMPLEMENTATION */
