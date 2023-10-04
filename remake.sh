#!/usr/bin/env sh
# For ease of typing.

make clean
exec make "-Bj$(($(nproc --all) + 1))" "${@}"
