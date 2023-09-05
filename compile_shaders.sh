#!/usr/bin/env sh
set -e

SHADER_COMPILE="glslc --target-env=vulkan1.2 $@"

cd shaders/
for shader in *.glsl; do
	printf "Compiling %s.\n" "${shader}"
	${SHADER_COMPILE} "${shader}" -o "$(basename "${shader}" .glsl).spv"
done
