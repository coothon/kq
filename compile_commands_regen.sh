#!/usr/bin/env sh
# Run this after changing debug CFLAGS or adding a new source file.
# It generates a file which configures clangd to use the debug build CFLAGS.

make clean
bear -- make "-Bj$(($(nproc) + 1))" kq_dbg
exec make clean
