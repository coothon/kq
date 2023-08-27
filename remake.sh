#!/usr/bin/env sh

export LSAN_OPTIONS=suppressions=lsan.supp
make clean
exec make "-Bj$(($(nproc --all) + 1))"
