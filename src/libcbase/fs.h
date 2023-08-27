// Include like a normal header wherever needed.
// And to provide the implementation, put
// #define LIBCBASE_FS_IMPLEMENTATION
// before including it in one translation unit.

#ifndef LIBCBASE_FS_H_
#define LIBCBASE_FS_H_

#include "common.h"

#include <stdbool.h>
#include <stdio.h>

extern bool fs_exists(const char fpath[static 1]);
extern bool fs_exists_regular(const char fpath[static 1]);
extern bool fs_exists_directory(const char fpath[static 1]);

extern off_t fs_file_size(const char fpath[static 1]);
extern off_t fs_file_sizef(FILE f[static 1]);

extern ulong fs_file_read(const char fpath[restrict static 1], size_t size, size_t n, char buf[restrict size * n]);
extern void *fs_file_read_all_alloc(const char fpath[restrict static 1], size_t size[restrict static 1]);

extern ulong fs_file_write(const char fpath[restrict static 1],
                           size_t     size,
                           size_t     n,
                           const char buf[restrict size * n]);
extern ulong fs_file_append(const char fpath[restrict static 1],
                            size_t     size,
                            size_t     n,
                            const char buf[restrict size * n]);
extern ulong fs_file_write_string(const char fpath[restrict static 1], size_t len, const char str[restrict len]);
extern ulong fs_file_write_cstring(const char fpath[restrict static 1], const char str[restrict static 1]);
extern ulong fs_file_append_string(const char fpath[restrict static 1], size_t len, const char str[restrict len]);
extern ulong fs_file_append_cstring(const char fpath[restrict static 1], const char str[restrict static 1]);

#endif /* LIBCBASE_FS_H_ */



#ifdef LIBCBASE_FS_IMPLEMENTATION
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

bool fs_exists(const char fpath[static 1]) {
	struct stat sb;
	return !stat(fpath, &sb);
}

bool fs_exists_regular(const char fpath[static 1]) {
	struct stat sb;
	return !stat(fpath, &sb) && S_ISREG(sb.st_mode);
}

bool fs_exists_directory(const char fpath[static 1]) {
	struct stat sb;
	return !stat(fpath, &sb) && S_ISDIR(sb.st_mode);
}

off_t fs_file_size(const char fpath[static 1]) {
	FILE *f = fopen(fpath, "rb");
	if (!f)
		return 0;

	fseeko(f, 0, SEEK_END);
	off_t sz = ftello(f);

	fclose(f);
	return sz;
}

off_t fs_file_sizef(FILE f[static 1]) {
	off_t orig_pos = ftello(f);

	fseeko(f, 0, SEEK_END);
	off_t sz = ftello(f);

	fseeko(f, orig_pos, SEEK_SET);
	clearerr(f);

	return sz;
}

ulong fs_file_read(const char fpath[restrict static 1], size_t size, size_t n, char buf[restrict size * n]) {
	FILE *f = fopen(fpath, "rb");
	if (!f)
		return 0;

	setvbuf(f, 0, _IOFBF, BUFSIZ);
	ulong c = fread(buf, size, n, f);

	fclose(f);
	return c;
}

void *fs_file_read_all_alloc(const char fpath[restrict static 1], size_t size[restrict static 1]) {
	FILE *f = fopen(fpath, "rb");
	if (!f)
		return 0;

	*size     = (size_t)fs_file_sizef(f);
	void *buf = malloc(*size);
	if (!buf) {
		fclose(f);
		return 0;
	}

	setvbuf(f, 0, _IOFBF, BUFSIZ);
	fread(buf, 1, *size, f);

	fclose(f);
	return buf;
}

ulong fs_file_write(const char fpath[restrict static 1], size_t size, size_t n, const char buf[restrict size * n]) {
	FILE *f = fopen(fpath, "wb");
	if (!f)
		return 0;

	setvbuf(f, 0, _IOFBF, BUFSIZ);
	ulong c = fwrite(buf, size, n, f);

	fclose(f);
	return c;
}

ulong fs_file_append(const char fpath[restrict static 1], size_t size, size_t n, const char buf[restrict size * n]) {
	FILE *f = fopen(fpath, "ab");
	if (!f)
		return 0;

	setvbuf(f, 0, _IOFBF, BUFSIZ);
	ulong c = fwrite(buf, size, n, f);

	fclose(f);
	return c;
}

ulong fs_file_write_string(const char fpath[restrict static 1], size_t len, const char str[restrict len]) {
	return fs_file_write(fpath, 1, len, str);
}

ulong fs_file_write_cstring(const char fpath[restrict static 1], const char str[restrict static 1]) {
	return fs_file_write_string(fpath, strlen(str), str);
}

ulong fs_file_append_string(const char fpath[restrict static 1], size_t len, const char str[restrict len]) {
	return fs_file_append(fpath, 1, len, str);
}

ulong fs_file_append_cstring(const char fpath[restrict static 1], const char str[restrict static 1]) {
	return fs_file_append_string(fpath, strlen(str), str);
}

#undef LIBCBASE_FS_IMPLEMENTATION
#endif /* LIBCBASE_FS_IMPLEMENTATION */
