#include <stdio.h>
#include <stdlib.h>

#include <glad/vulkan.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <kq.h>
#include <libcbase/log.h>


#define CB_LOG_MODULE "KQ"


static kq_data kq = {0};


int main(void) {
	setvbuf(stderr, 0, _IOLBF, BUFSIZ);
	cb_log_init(stderr, CB_LOG_LEVEL_TRACE, false, false);
	cb_log_infer_use_colours();
	KQinit(&kq);

	while (!glfwWindowShouldClose(kq.win)) {
		glfwPollEvents();

		if (!KQrender_begin(&kq))
			break;

		if (!KQdraw_quad(&kq, (vec2){0.0f, 0.0f}, (vec2){1.0f, 1.0f}))
			break;
		if (!KQdraw_quad(&kq, (vec2){1.0f, 0.0f}, (vec2){1.0f, 1.0f}))
			break;
		if (!KQdraw_quad(&kq, (vec2){-1.0f, 0.0f}, (vec2){1.0f, 1.0f}))
			break;

		if (!KQrender_end(&kq))
			break;
	}

	KQstop(&kq);
	return EXIT_SUCCESS;
}
