// Include like a normal header wherever needed.
// And to provide the implementation, put
// #define LIBCBASE_ARENA_IMPLEMENTATION
// before including it in one translation unit.

#ifndef LIBCBASE_ARENA_H_
#define LIBCBASE_ARENA_H_

#include <stdlib.h>

#include "common.h"

#define CB_ARENA_DEFAULT_ALIGN (sizeof(void *[2]))

typedef struct cb_arena {
	// You can't do pointer arithmetic with void pointers.
	uchar       *start;
	const uchar *cursor;
	const uchar *end;
} cb_arena;

typedef struct cb_arena_save_point {
	const uchar *cursor;
} cb_arena_save_point;

// Always returns the `arena` pointer. Success is indicated if both the returned value and its `start` member are non-null.
// Example usage: cb_arena *my_arena = cb_arena_new(malloc(sizeof(cb_arena)), 512);
// Note: Ensure you differentiate between freeing the arena's memory and the arena struct itself.
extern cb_arena *cb_arena_new(cb_arena *arena, size_t capacity);
extern cb_arena *cb_arena_new_zeroed(cb_arena *arena, size_t capacity);

extern void cb_arena_destroy(cb_arena arena[static 1]);

extern void cb_arena_clear(cb_arena arena[static 1]);
extern void cb_arena_clear_zero(cb_arena arena[static 1]);

// Returns a pointer to a valid memory region within the arena, lasting for the arena's lifetime.
// No manual deallocation required.
extern void *cb_arena_alloc(cb_arena arena[static 1], size_t size);
extern void *cb_arena_alloc_align(cb_arena arena[static 1], size_t size, size_t align);

extern size_t cb_arena_space_left(const cb_arena arena[static 1]);

// Use these to revert to a previous arena state, invalidating all allocations since.
extern void cb_arena_save(const cb_arena arena[restrict static 1], cb_arena_save_point save_point[restrict static 1]);
extern void cb_arena_load(cb_arena arena[restrict static 1], const cb_arena_save_point save_point[restrict static 1]);

#endif /* LIBCBASE_ARENA_H_ */



#ifdef LIBCBASE_ARENA_IMPLEMENTATION
#include "log.h"

#include <string.h>
#include <stdbool.h>

// <https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/>
static __always_inline bool is_power_of_two(uintptr_t x);
static const uchar         *align_forward(const uchar ptr[static 1], size_t align);

cb_arena *cb_arena_new(cb_arena *arena, size_t capacity) {
	if (arena) {
		LOG_TRACE("New arena: Capacity: %zu.", capacity);
		register uchar *arena_buf = capacity ? malloc(capacity) : 0;

		*arena = (cb_arena){
			.start  = arena_buf,
			.cursor = arena_buf,
			.end    = arena_buf + capacity,
		};
	}

	return arena;
}

cb_arena *cb_arena_new_zeroed(cb_arena *arena, size_t capacity) {
	if (arena) {
		LOG_TRACE("New arena (zeroed): Capacity: %zu.", capacity);
		register uchar *arena_buf = capacity ? calloc(capacity, 1) : 0;

		*arena = (cb_arena){
			.start  = arena_buf,
			.cursor = arena_buf,
			.end    = arena_buf + capacity,
		};
	}

	return arena;
}

void cb_arena_destroy(cb_arena arena[static 1]) {
	free(arena->start);
}

void cb_arena_clear(cb_arena arena[static 1]) {
	arena->cursor = arena->start;
}

void cb_arena_clear_zero(cb_arena arena[static 1]) {
	memset(arena->start, 0, (size_t)(arena->cursor - arena->start));
	arena->cursor = arena->start;
}

void *cb_arena_alloc(cb_arena arena[static 1], size_t size) {
	return cb_arena_alloc_align(arena, size, CB_ARENA_DEFAULT_ALIGN);
}

void *cb_arena_alloc_align(cb_arena arena[static 1], size_t size, size_t align) {
	register const uchar *new_cursor = (uchar *)align_forward(arena->cursor, align);
	register const uchar *new_end    = new_cursor + size;
	if (new_end > arena->end)
		return 0;

	LOG_TRACE("Arena alloc: Size: %zu, Align: %zu, Padding: %zu", size, align, (size_t)(new_cursor - arena->cursor));

	arena->cursor = new_end;
	return (void *)new_cursor;
}

size_t cb_arena_space_left(const cb_arena arena[static 1]) {
	return (size_t)(arena->end - arena->cursor);
}

void cb_arena_save(const cb_arena arena[restrict static 1], cb_arena_save_point save_point[restrict static 1]) {
	save_point->cursor = arena->cursor;
}

void cb_arena_load(cb_arena arena[restrict static 1], const cb_arena_save_point save_point[restrict static 1]) {
	arena->cursor = save_point->cursor;
}



static __always_inline bool is_power_of_two(uintptr_t x) {
	return x ? !(x & (x - 1)) : false;
}

static const uchar *align_forward(const uchar ptr[static 1], size_t align) {
	if (align) {
		register uintptr_t modulo;
		if (is_power_of_two(align)) {
			modulo = (uintptr_t)ptr & (align - 1);
		} else {
			LOG_DEBUG("Requested align is not a power of two! "
			          "(%zu)",
			          align);
			modulo = (uintptr_t)ptr % align;
		}

		if (modulo)
			ptr += align - modulo;
	} else {
		LOG_DEBUG("Requested align is 0!");
	}

	return ptr;
}

#undef LIBCBASE_ARENA_IMPLEMENTATION
#endif /* LIBCBASE_ARENA_IMPLEMENTATION */
