#!/usr/bin/env sh
# For ease of typing.

exec make "-j$(($(nproc --all) + 1))" "${@}"
