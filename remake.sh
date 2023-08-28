#!/usr/bin/env sh

make clean
exec make "-Bj$(($(nproc --all) + 1))"
