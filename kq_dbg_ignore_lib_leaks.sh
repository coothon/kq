#!/usr/bin/env sh
# Run this instead of ./kq_dbg.

exec env LSAN_OPTIONS=suppressions=lsan.supp ./kq_dbg
