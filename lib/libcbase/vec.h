#ifndef LIBCBASE_VEC_H_
#define LIBCBASE_VEC_H_

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define CB_VEC_DEFAULT_CAP 8

#define cb_mk_vec(name, type)                                                                                         \
	typedef struct name {                                                                                         \
		size_t size;                                                                                          \
		size_t cap;                                                                                           \
		type  *p;                                                                                             \
	} name;                                                                                                       \
                                                                                                                      \
	extern name  *name##_create(size_t initial_cap);                                                              \
	extern name  *name##_create_zeroed(size_t initial_cap);                                                       \
	extern void   name##_destroy(name vec[static 1]);                                                             \
	extern size_t name##_resize(name vec[static 1], size_t new_size);                                             \
	extern size_t name##_realloc(name vec[static 1], size_t new_cap);                                             \
	extern type  *name##_push_back(name vec[restrict static 1], const type val[restrict static 1]);               \
	extern void   name##_clear(name vec[static 1]);                                                               \
	extern bool   name##_insert(name vec[restrict static 1], size_t at_index, const type val[restrict static 1]); \
	extern bool   name##_push(name vec[restrict static 1], const type val[restrict static 1]);



#define cb_impl_vec(name, type)                                                                               \
	static __always_inline size_t name##_new_cap(size_t old_cap) {                                        \
		return old_cap * 2;                                                                           \
	}                                                                                                     \
                                                                                                              \
	name *name##_create(size_t initial_cap) {                                                             \
		if (!initial_cap)                                                                             \
			initial_cap = CB_VEC_DEFAULT_CAP;                                                     \
                                                                                                              \
		name *vec = malloc(sizeof(name));                                                             \
		if (!vec)                                                                                     \
			return 0;                                                                             \
                                                                                                              \
		vec->p = malloc(sizeof(type[initial_cap]));                                                   \
		if (!vec->p) {                                                                                \
			free(vec);                                                                            \
			return 0;                                                                             \
		}                                                                                             \
                                                                                                              \
		vec->size = 0;                                                                                \
		vec->cap = initial_cap;                                                                       \
                                                                                                              \
		return vec;                                                                                   \
	}                                                                                                     \
                                                                                                              \
	name *name##_create_zeroed(size_t initial_cap) {                                                      \
		if (!initial_cap)                                                                             \
			initial_cap = CB_VEC_DEFAULT_CAP;                                                     \
                                                                                                              \
		name *vec = malloc(sizeof(name));                                                             \
		if (!vec)                                                                                     \
			return 0;                                                                             \
                                                                                                              \
		vec->p = calloc(initial_cap, sizeof(type));                                                   \
		if (!vec->p) {                                                                                \
			free(vec);                                                                            \
			return 0;                                                                             \
		}                                                                                             \
                                                                                                              \
		vec->size = 0;                                                                                \
		vec->cap = initial_cap;                                                                       \
                                                                                                              \
		return vec;                                                                                   \
	}                                                                                                     \
                                                                                                              \
	void name##_destroy(name vec[static 1]) {                                                             \
		free(vec->p);                                                                                 \
		free(vec);                                                                                    \
	}                                                                                                     \
                                                                                                              \
	size_t name##_resize(name vec[static 1], size_t new_size) {                                           \
		if (new_size <= vec->cap) {                                                                   \
			vec->size = new_size;                                                                 \
			return vec->cap;                                                                      \
		}                                                                                             \
                                                                                                              \
		size_t result = name##_realloc(vec, new_size);                                                \
		if (result)                                                                                   \
			vec->size = vec->cap;                                                                 \
		return result;                                                                                \
	}                                                                                                     \
                                                                                                              \
	size_t name##_realloc(name vec[static 1], size_t new_cap) {                                           \
		if (!new_cap)                                                                                 \
			new_cap = CB_VEC_DEFAULT_CAP;                                                         \
                                                                                                              \
		type *tmp_vec_p = reallocarray(vec->p, new_cap, sizeof(type));                                \
		if (!tmp_vec_p)                                                                               \
			return 0;                                                                             \
                                                                                                              \
		register size_t tmp_vec_cap = vec->cap;                                                       \
                                                                                                              \
		vec->p = tmp_vec_p;                                                                           \
		vec->cap = new_cap;                                                                           \
		if (new_cap < vec->size)                                                                      \
			vec->size = new_cap;                                                                  \
                                                                                                              \
		return tmp_vec_cap;                                                                           \
	}                                                                                                     \
                                                                                                              \
	type *name##_push_back(name vec[restrict static 1], const type val[restrict static 1]) {              \
		if (vec->size >= vec->cap)                                                                    \
			if (!name##_realloc(vec, name##_new_cap(vec->cap)))                                   \
				return 0;                                                                     \
                                                                                                              \
		memcpy(&vec->p[vec->size], val, sizeof(type));                                                \
		return &vec->p[vec->size++];                                                                  \
	}                                                                                                     \
                                                                                                              \
	void name##_clear(name vec[static 1]) {                                                               \
		vec->size = 0;                                                                                \
	}                                                                                                     \
                                                                                                              \
	bool name##_insert(name vec[restrict static 1], size_t at_index, const type val[restrict static 1]) { \
		if (at_index > vec->size)                                                                     \
			return false;                                                                         \
                                                                                                              \
		/* Same as push_back. */                                                                      \
		if (at_index == vec->size)                                                                    \
			return name##_push_back(vec, val);                                                    \
                                                                                                              \
		register size_t size_to_move = ++vec->size - at_index;                                        \
		if (vec->size >= vec->cap)                                                                    \
			if (!name##_realloc(vec, name##_new_cap(vec->cap)))                                   \
				return false;                                                                 \
                                                                                                              \
		memmove(&vec->p[at_index + 1], &vec->p[at_index], sizeof(type[size_to_move]));                \
		memcpy(&vec->p[at_index], val, sizeof(type));                                                 \
		return true;                                                                                  \
	}                                                                                                     \
                                                                                                              \
	bool name##_push(name vec[restrict static 1], const type val[restrict static 1]) {                    \
		register size_t size_to_move = ++vec->size;                                                   \
		if (vec->size >= vec->cap)                                                                    \
			if (!name##_realloc(vec, name##_new_cap(vec->cap)))                                   \
				return false;                                                                 \
                                                                                                              \
		memmove(&vec->p[1], vec->p, sizeof(type[size_to_move]));                                      \
		memcpy(vec->p, val, sizeof(type));                                                            \
		return true;                                                                                  \
	}

#define cb_vec(name, type) cb_mk_vec(name, type) cb_impl_vec(name, type)

#endif /* LIBCBASE_VEC_H_ */
