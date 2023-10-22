# Configuration:
PROJ:=kq

CC:=clang
AR:=llvm-ar
STRIP:=llvm-strip

# Libraries against which to link.
LIBS:=
LDFILES:=$(shell pkg-config --static --libs $(LIBS) 2>/dev/null) -lm
LDFLAGS:=$(shell pkg-config --cflags $(LIBS) 2>/dev/null)

# Turn on every warning, and then remove the ones I don't care about.
WARNS:=-Weverything -pedantic \
-Wno-unused-command-line-argument -Wno-incompatible-pointer-types-discards-qualifiers -Wno-extra-semi -Wno-unsafe-buffer-usage -Wno-cast-function-type-strict -Wno-undef -Wno-padded -Wno-pre-c23-compat -Wno-extra-semi-stmt -Wno-declaration-after-statement -Wno-unused-macros -Wno-vla -Wno-implicit-fallthrough -Wno-cast-qual -Wno-switch-enum -Wno-double-promotion -Wno-documentation -Wno-documentation-unknown-command

BUILD_DIR:=build
SRC_DIRS:=src lib

SRCS:=$(shell find -O3 $(SRC_DIRS) -not -path '*/glfw/*' -a -name '*.c')
ANALYZE_SRCS:=$(shell find -O3 $(SRC_DIRS) -not -path '*/glfw/*' -a -name '*.[ch]')

OBJS_DEBUG:=$(SRCS:%=$(BUILD_DIR)/%.dbg.o)
OBJS_RELEASE:=$(SRCS:%=$(BUILD_DIR)/%.rel.o)

# Feature test macros needed to compile.
CPPFLAGS_COMMON:=-D_DEFAULT_SOURCE=1 -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE=1 -DVK_USE_PLATFORM_WAYLAND_KHR=1 -Ilib -Isrc -Ilib/glfw/include
CPPFLAGS_DEBUG:=-UNDEBUG -DDEBUG=1 -DCB_DEBUG=1 -DKQ_DEBUG=1 -DCB_LOG_LEVEL_COMPILE_TIME_MIN=CB_LOG_LEVEL_TRACE
CPPFLAGS_RELEASE:=-DNDEBUG=1 -UDEBUG -UCB_DEBUG -UKQ_DEBUG -DCB_LOG_LEVEL_COMPILE_TIME_MIN=CB_LOG_LEVEL_WARN

CFLAGS_COMMON:=$(LDFLAGS) $(CPPFLAGS_COMMON) $(WARNS) -std=c23 -pipe -fuse-ld=lld -fwrapv -march=native -mtune=native -fpie
CFLAGS_DEBUG:=$(CFLAGS_COMMON) $(CPPFLAGS_DEBUG) -glldb -gdwarf-5 -gdwarf64 -rdynamic -O0 -fsanitize=address,undefined -fsanitize-trap=all -ftrapv -fno-omit-frame-pointer -fno-optimize-sibling-calls
CFLAGS_RELEASE:=$(CFLAGS_COMMON) $(CPPFLAGS_RELEASE) -g0 -s -Ofast -ffast-math -fomit-frame-pointer -flto=full

# Controls which compiled executable will be foremost.
COMPILE_MODE:=dbg


debug: dbg
dbg: COMPILE_MODE:=dbg
dbg: all
	@./compile_shaders.sh -O0 -g

release: rel
rel: COMPILE_MODE:=rel
rel: all
	@./compile_shaders.sh -O

all: $(PROJ) $(PROJ)_dbg $(PROJ)_rel


$(PROJ): $(PROJ)_$(COMPILE_MODE)
	@-printf "LN\t%s -> %s\n" "$<" "$@"
	@ln "$<" "$@"

$(PROJ)_dbg: $(OBJS_DEBUG) libglfw3.dbg.a
	@-printf "LD\t%s\n" "$@"
	@"$(CC)" $(CFLAGS_DEBUG) $(LDFILES) -pie $^ -o "$@"

$(PROJ)_rel: $(OBJS_RELEASE) libglfw3.rel.a
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

libglfw3.rel.a:
	-rm -rf lib/glfw/build.rel
	mkdir lib/glfw/build.rel
	cmake -S lib/glfw -B lib/glfw/build.rel \
		-G Ninja \
		-D BUILD_SHARED_LIBS=OFF \
		-D GLFW_BUILD_EXAMPLES=OFF \
		-D GLFW_BUILD_TESTS=OFF \
		-D GLFW_BUILD_DOCS=OFF \
		-D GLFW_VULKAN_STATIC=ON \
		-D GLFW_USE_WAYLAND=ON \
		-D CMAKE_C_COMPILER="$(CC)" \
		-D CMAKE_AR="$(shell which "$(AR)")" \
		-D CMAKE_C_FLAGS="$(CFLAGS_RELEASE) -fno-pie -no-pie -fpic -Wno-everything"
	cmake --build lib/glfw/build.rel
	cp lib/glfw/build.rel/src/libglfw3.a "$@"

libglfw3.dbg.a:
	-rm -rf lib/glfw/build.dbg
	mkdir lib/glfw/build.dbg
	cmake -S lib/glfw -B lib/glfw/build.dbg \
		-G Ninja \
		-D BUILD_SHARED_LIBS=OFF \
		-D GLFW_BUILD_EXAMPLES=OFF \
		-D GLFW_BUILD_TESTS=OFF \
		-D GLFW_BUILD_DOCS=OFF \
		-D GLFW_VULKAN_STATIC=ON \
		-D GLFW_USE_WAYLAND=ON \
		-D CMAKE_C_COMPILER="$(CC)" \
		-D CMAKE_AR="$(shell which "$(AR)")" \
		-D CMAKE_C_FLAGS="$(CFLAGS_DEBUG) -fno-pie -no-pie -fpic -Wno-everything"
	cmake --build lib/glfw/build.dbg
	cp lib/glfw/build.dbg/src/libglfw3.a "$@"

analyze:
	-clang-tidy $(ANALYZE_SRCS) -- $(CFLAGS_DEBUG)

clean:
	-rm -rf "$(PROJ)_dbg" "$(PROJ)_rel" "$(PROJ)" "$(BUILD_DIR)"

clean_glfw:
	-rm -rf libglfw3.dbg.a libglfw3.rel.a lib/glfw/build.dbg lib/glfw/build.rel

.PHONY: all debug dbg release rel analyze clean clean_glfw
