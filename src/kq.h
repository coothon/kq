#ifndef KQ_H_
#define KQ_H_

#include <stdbool.h>

#include "glad/vulkan.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

typedef struct kq_data {
	GLFWwindow *win;

	VkInstance vk_ins;

	int vk_ver;
} kq_data;

extern bool KQinit(kq_data kq[static 1]);
extern void KQstop(kq_data kq[static 1]);

#endif /* KQ_H_ */
