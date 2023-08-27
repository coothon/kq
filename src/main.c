#include <stdlib.h>

#define CB_LOG_MODULE "kq"

#define GLAD_VULKAN_IMPLEMENTATION
#include "glad/vulkan.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "kq.h"
#define LIBCBASE_LOG_IMPLEMENTATION
#include "libcbase/log.h"

static void kq_glfw_callback_error(int e, const char *desc);

int main(void) {
	cb_log_init(CB_LOG_LEVEL_TRACE);

	glfwSetErrorCallback(kq_glfw_callback_error);
	if (!glfwInit()) {
		LOG_MFATAL("glfwInit() failed.");
		return EXIT_FAILURE;
	}
	LOG_MTRACE("GLFW initialized.");

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
	glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	GLFWwindow *win = glfwCreateWindow(800, 600, "kq", 0, 0);
	if (!win) {
		LOG_MFATAL("glfwCreateWindow() failed.");
		glfwTerminate();
		return EXIT_FAILURE;
	}
	LOG_MTRACE("GLFW window created.");

	int vk_version = gladLoaderLoadVulkan(0, 0, 0);
	if (!vk_version) {
		LOG_MFATAL("gladLoaderLoadVulkan() failed.");
		glfwDestroyWindow(win);
		glfwTerminate();
		return EXIT_FAILURE;
	}
	LOG_MTRACE("Vulkan loaded.");

	gladInstallVulkanDebug();


	gladUninstallVulkanDebug();
	gladLoaderUnloadVulkan();
	glfwDestroyWindow(win);
	glfwTerminate();
	return EXIT_SUCCESS;
}

static void kq_glfw_callback_error(int e, const char *desc) {
	LOG_MERROR("GLFW (error %x): %s", e, desc);
}
