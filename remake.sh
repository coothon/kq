#!/usr/bin/env sh
# For ease of typing.

make clean
exec make "-j$(($(nproc --all) + 1))" "${@}"
