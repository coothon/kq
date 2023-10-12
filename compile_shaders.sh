#!/usr/bin/env sh
set -e

cd shaders/

# Select shader source files.
find -O3 . -type f -a '(' -name '*.vert*' -o -name '*.frag*' ')' -a '!' -name '*.spv*' | while read -r shader; do
	printf "GLSLC\t%s\n" "${shader}"
	glslc --target-env=vulkan1.3 "${@}" "${shader}" -o "${shader}.spv"
done
