#ifndef LIBCBASE_LIST_H_
#define LIBCBASE_LIST_H_

#define cb_mk_list(name, type)                                       \
	typedef struct name name;                                    \
	struct name {                                                \
		name *next;                                          \
		name *prev;                                          \
		type  usr;                                           \
	};                                                           \
                                                                     \
	extern name *name##_get_start(name l[static 1]);             \
	extern name *name##_get_end(name l[static 1]);               \
	extern name *name##_insert_before(name l[restrict static 1], \
	                                  name *restrict to_insert); \
	extern name *name##_insert_after(name l[restrict static 1],  \
	                                 name *restrict to_insert);  \
	extern name *name##_at_index(name l[restrict static 1], long index);

#define cb_impl_list(name, type)                                       \
	name *name##_get_start(name l[static 1]) {                     \
		for (name *p = l->prev; p; (l = p), (p = p->prev))     \
			;                                              \
		return l;                                              \
	}                                                              \
                                                                       \
	name *name##_get_end(name l[static 1]) {                       \
		for (name *p = l->next; p; (l = p), (p = p->next))     \
			;                                              \
		return l;                                              \
	}                                                              \
                                                                       \
	name *name##_insert_before(name l[restrict static 1],          \
	                           name *restrict to_insert) {         \
		name *ret = l->prev;                                   \
		l->prev   = to_insert;                                 \
		return ret;                                            \
	}                                                              \
                                                                       \
	name *name##_insert_after(name l[restrict static 1],           \
	                          name *restrict to_insert) {          \
		name *ret = l->next;                                   \
		l->next   = to_insert;                                 \
		return ret;                                            \
	}                                                              \
                                                                       \
	name *name##_at_index(name l[restrict static 1], long index) { \
		if (index > 0L) {                                      \
			for (long i = 0L; i < index; ++i) {            \
				l = l->next;                           \
				if (!l)                                \
					return 0;                      \
			}                                              \
		} else {                                               \
			for (long i = 0L; i > index; --i) {            \
				l = l->prev;                           \
				if (!l)                                \
					return 0;                      \
			}                                              \
		}                                                      \
		return l;                                              \
	}

#define cb_list(name, type) cb_mk_list(name, type) cb_impl_list(name, type)

#endif /* LIBCBASE_LIST_H_ */
