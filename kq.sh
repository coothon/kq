#!/usr/bin/env sh
# Run this instead of ./kq.

exec env LSAN_OPTIONS=suppressions=lsan.supp ./kq
