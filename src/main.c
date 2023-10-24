#include "freetype/freetype.h"
#include <stdio.h>
#include <stdlib.h>

#include <glad/vulkan.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <hb.h>
#include <hb-ft.h>
#include <hb-icu.h>

#include <ft2build.h>
#include FT_FREETYPE_H

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

	FT_Face ft_face = {0};
	e = FT_New_Face(ft2_lib, KQTXT_FONT, 0, &ft_face);
	if (e) {
		LOGM_FATAL("Failed loading font: %s.", FT_Error_String(e));
		FT_Done_FreeType(ft2_lib);
		KQstop(&kq);
		return EXIT_FAILURE;
	}

	FT_Set_Char_Size(ft_face, 0, 16*64, 300, 300); // Tutorial values.

	hb_unicode_funcs_t *icufuncs;
	icufuncs = hb_icu_get_unicode_funcs();

	hb_buffer_t *buf;
	buf = hb_buffer_create();
	hb_buffer_set_unicode_funcs(buf, icufuncs);
	hb_buffer_add_utf8(buf, "Hello, world!", -1, 0, -1);
	hb_buffer_set_direction(buf, HB_DIRECTION_LTR);
	hb_buffer_set_script(buf, HB_SCRIPT_LATIN);
	hb_buffer_set_language(buf, hb_language_from_string("en", -1));

	hb_font_t *font = hb_ft_font_create_referenced(ft_face);
	hb_ft_font_set_funcs(font);

	hb_shape(font, buf, 0, 0);

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

	hb_font_destroy(font);
	hb_buffer_destroy(buf);
	FT_Done_FreeType(ft2_lib);
	KQstop(&kq);
	return EXIT_SUCCESS;
}
