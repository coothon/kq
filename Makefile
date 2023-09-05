# Configuration:
PROJ_NAME:=kq

CC:=clang
AR:=llvm-ar
STRIP:=llvm-strip

# Libraries against which to link.
LIBS:=glfw3
LDFILES:=$(shell pkg-config --static --libs $(LIBS) 2>/dev/null)
LDFLAGS:=$(shell pkg-config --cflags $(LIBS) 2>/dev/null)

# Turn on every warning, and then remove the ones I don't care about.
WARNS:=-Wall -Wextra -Wshadow -Wunreachable-code -Wconversion -Wsign-conversion -Wformat -Wmissing-braces -Wparentheses -Wreserved-identifier -pedantic \
-Wno-unused-command-line-argument -Wno-incompatible-pointer-types-discards-qualifiers -Wno-extra-semi

BUILD_DIR:=build
SRC_DIRS:=src

SRCS:=$(shell find -O3 $(SRC_DIRS) -name '*.c')
ANALYZE_SRCS:=$(shell find -O3 $(SRC_DIRS) -name '*.[ch]')

OBJS_DEBUG:=$(SRCS:%=$(BUILD_DIR)/%.dbg.o)
OBJS_RELEASE:=$(SRCS:%=$(BUILD_DIR)/%.rel.o)

# Feature test macros needed to compile.
CPPFLAGS_COMMON:=-D_DEFAULT_SOURCE=1 -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE=1
CPPFLAGS_DEBUG:=-UNDEBUG -DDEBUG=1 -DCB_DEBUG=1 -DKQ_DEBUG=1
CPPFLAGS_RELEASE:=-DNDEBUG=1 -UDEBUG -UCB_DEBUG -UKQ_DEBUG

CFLAGS_COMMON:=$(LDFLAGS) $(CPPFLAGS_COMMON) $(WARNS) -std=c17 -pipe -fuse-ld=lld -fwrapv -march=native -mtune=native
CFLAGS_DEBUG:=$(CFLAGS_COMMON) $(CPPFLAGS_DEBUG) -glldb -rdynamic -O0 -fsanitize=address,undefined -fno-sanitize-trap=all -ftrapv -fno-omit-frame-pointer -fno-optimize-sibling-calls
CFLAGS_RELEASE:=$(CFLAGS_COMMON) $(CPPFLAGS_RELEASE) -g0 -Ofast -ffast-math -fomit-frame-pointer -flto=full



all: kq_dbg kq_rel
	@./compile_shaders.sh -O0 -g

kq_dbg: $(OBJS_DEBUG)
	@-printf "LD\t%s\n" "$@"
	@"$(CC)" $(CFLAGS_DEBUG) $(LDFILES) -pie $^ -o "$@"

kq_rel: $(OBJS_RELEASE)
	@-printf "LD\t%s\n" "$@"
	@"$(CC)" $(CFLAGS_RELEASE) $(LDFILES) -pie $^ -o "$@"
	@-printf "STRIP\t%s\n" "$@"
	@"$(STRIP)" -sx --strip-sections "$@"

# Debug objects.
$(BUILD_DIR)/%.c.dbg.o: %.c
	@-printf "CC\t%s\n" "$@"
	@mkdir -p $(dir $@)
	@"$(CC)" $(CFLAGS_DEBUG) -c "$<" -o "$@"

# Release objects.
$(BUILD_DIR)/%.c.rel.o: %.c
	@-printf "CC\t%s\n" "$@"
	@mkdir -p $(dir $@)
	@"$(CC)" $(CFLAGS_RELEASE) -c "$<" -o "$@"

analyze:
	-clang-tidy $(ANALYZE_SRCS) -- $(CFLAGS_DEBUG)

clean:
	-rm -rf kq_dbg kq_rel $(BUILD_DIR)

.PHONY: all analyze clean
