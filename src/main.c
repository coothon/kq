#include "freetype/freetype.h"
#include <stdio.h>
#include <stdlib.h>

#include <glad/vulkan.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <kq.h>
#include <libcbase/log.h>


#define CB_LOG_MODULE "KQ"


static kq_data kq = {0};

static FT_Library ft2_lib = {0};

int main(void) {
	setvbuf(stderr, 0, _IOLBF, BUFSIZ);
	cb_log_init(stderr, CB_LOG_LEVEL_TRACE, false, false);
	cb_log_infer_use_colours();
	KQinit(&kq);

	FT_Error e = FT_Init_FreeType(&ft2_lib);
	if (e) {
		LOGM_FATAL("Error during freetype initialization: %s.", FT_Error_String(e));
		KQstop(&kq);
		return EXIT_FAILURE;
	}

	while (!glfwWindowShouldClose(kq.win)) {
		glfwPollEvents();

		if (!KQrender_begin(&kq))
			break;

		if (!KQdraw_quad(&kq, (vec2){-0.5f, 0.0f}, (vec2){1.0f, 1.0f}, 0))
			break;
		if (!KQdraw_quad(&kq, (vec2){0.5f, 0.0f}, (vec2){1.0f, 1.0f}, 1))
			break;

		if (!KQrender_end(&kq))
			break;
	}

	FT_Done_FreeType(ft2_lib);
	KQstop(&kq);
	return EXIT_SUCCESS;
}
