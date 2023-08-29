#include <stdlib.h>

#define CB_LOG_MODULE "KQ"

#define GLAD_VULKAN_IMPLEMENTATION
#include "glad/vulkan.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "kq.h"
#define LIBCBASE_LOG_IMPLEMENTATION
#include "libcbase/log.h"

static kq_data kq = {0};

int main(void) {
	cb_log_init(CB_LOG_LEVEL_TRACE, false);
	KQinit(&kq);

	LOG_DEBUG("KQ initialized.");

	KQstop(&kq);
	return EXIT_SUCCESS;
}
