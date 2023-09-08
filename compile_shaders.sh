#!/usr/bin/env sh
set -e

cd shaders/

OLD_IFS="${IFS}"
IFS='
'
for shader in $(find -O3 . -type f -a '(' -name '*vert*' -o -name '*frag*' ')' -a '!' -name '*spv*'); do
	IFS="${OLD_IFS}"
	printf "GLSLC\t%s\n" "${shader}"
	glslc --target-env=vulkan1.3 "${@}" "${shader}" -o "${shader}.spv"
done
