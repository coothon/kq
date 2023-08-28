#include "kq.h"

#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <string.h>

#include "libcbase/common.h"
#include "libcbase/log.h"
#include "libcbase/vec.h"

#define KQ_OOM_MSG() LOG_MFATAL("Out of memory! (OOM)")

#define CB_LOG_MODULE "KQ"

static void kq_callback_glfw_error(int e, const char *desc);


#if KQ_DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL kq_callback_vk_debug(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                           VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                                           const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                           void                                       *pUserData);

static bool kqvk_instance_add_validation_layers(VkInstanceCreateInfo instance_cinfo[static 1]);
#endif /* KQ_DEBUG */
static bool kqvk_instance_add_extensions(VkInstanceCreateInfo instance_cinfo[static 1]);

cb_vec(vecstr, char *);

#if KQ_DEBUG
static const char *const kqvk_wanted_validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
static const u32         kqvk_wanted_validation_layers_count =
	sizeof kqvk_wanted_validation_layers / sizeof kqvk_wanted_validation_layers[0];
vecstr *kqvk_validation_layers_vec = 0;
#endif /* KQ_DEBUG */
vecstr *kqvk_instance_exts_vec = 0;


bool KQinit(kq_data kq[static 1]) {
	glfwSetErrorCallback(kq_callback_glfw_error);
	if (!glfwInit()) {
		LOG_MFATAL("GLFW initialization failed.");
		return false;
	}
	LOG_MTRACE("GLFW initialized.");

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
	glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	kq->win = glfwCreateWindow(800, 600, "kq", 0, 0);
	if (!kq->win) {
		LOG_MFATAL("GLFW window creation failed.");
		glfwTerminate();
		return false;
	}
	LOG_MTRACE("GLFW window created.");

	glfwSetWindowUserPointer(kq->win, kq);

	kq->vk_ver = gladLoaderLoadVulkan(0, 0, 0);
	if (!kq->vk_ver) {
		LOG_MFATAL("GLAD Vulkan loader failed.");
		glfwDestroyWindow(kq->win);
		glfwTerminate();
		return false;
	}
	LOG_MTRACE("Vulkan loaded.");
	LOG_MINFO("Vulkan %d.%d.", GLAD_VERSION_MAJOR(kq->vk_ver), GLAD_VERSION_MINOR(kq->vk_ver));

	gladInstallVulkanDebug();

#if KQ_DEBUG
	VkDebugUtilsMessengerCreateInfoEXT dbg_messenger_cinfo = {
		.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
	                         | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
	                         | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
	                     | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
	                     | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = kq_callback_vk_debug};
#endif /* KQ_DEBUG */

	VkInstanceCreateInfo instance_cinfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
#if KQ_DEBUG
		.pNext = &dbg_messenger_cinfo
#endif /* KQ_DEBUG */
	};

#if KQ_DEBUG
	if (!kqvk_instance_add_validation_layers(&instance_cinfo)) {
		gladUninstallVulkanDebug();
		gladLoaderUnloadVulkan();
		glfwDestroyWindow(kq->win);
		glfwTerminate();
		return false;
	}
#endif /* KQ_DEBUG */

	if (!kqvk_instance_add_extensions(&instance_cinfo)) {
#if KQ_DEBUG
		vecstr_destroy(kqvk_validation_layers_vec);
#endif /* KQ_DEBUG */
		gladUninstallVulkanDebug();
		gladLoaderUnloadVulkan();
		glfwDestroyWindow(kq->win);
		glfwTerminate();
		return false;
	}

	if (vkCreateInstance(&instance_cinfo, 0, &kq->vk_ins)) {
		LOG_MFATAL("Creating VkInstance failed.");
		vecstr_destroy(kqvk_instance_exts_vec);
#if KQ_DEBUG
		vecstr_destroy(kqvk_validation_layers_vec);
#endif /* KQ_DEBUG */
		gladUninstallVulkanDebug();
		gladLoaderUnloadVulkan();
		glfwDestroyWindow(kq->win);
		glfwTerminate();
	}
	LOG_MTRACE("VkInstance created.");

	return true;
}

void KQstop(kq_data kq[static 1]) {
	vkDestroyInstance(kq->vk_ins, 0);
	vecstr_destroy(kqvk_instance_exts_vec);
#if KQ_DEBUG
	vecstr_destroy(kqvk_validation_layers_vec);
#endif /* KQ_DEBUG */
	gladUninstallVulkanDebug();
	gladLoaderUnloadVulkan();
	glfwDestroyWindow(kq->win);
	glfwTerminate();
}



#undef CB_LOG_MODULE
#define CB_LOG_MODULE "KQVK"
#if KQ_DEBUG
static bool kqvk_instance_add_validation_layers(VkInstanceCreateInfo instance_cinfo[static 1]) {
	u32 avail_layers_count = 0;
	vkEnumerateInstanceLayerProperties(&avail_layers_count, 0);
	VkLayerProperties *avail_layers = malloc(sizeof(VkLayerProperties[avail_layers_count]));
	if (!avail_layers) {
		KQ_OOM_MSG();
		return false;
	}

	kqvk_validation_layers_vec = vecstr_create(avail_layers_count);
	if (!kqvk_validation_layers_vec) {
		KQ_OOM_MSG();
		free(avail_layers);
		return false;
	}

	vkEnumerateInstanceLayerProperties(&avail_layers_count, avail_layers);
	for (u32 i = 0U; i < kqvk_wanted_validation_layers_count; ++i) {
		bool found = false;
		for (u32 j = 0U; j < avail_layers_count; ++j) {
			if (!strcmp(avail_layers[j].layerName, kqvk_wanted_validation_layers[i])) {
				found = true;
				break;
			}
		}

		if (found) {
			LOG_MDEBUG("Enabling validation layer %s.", kqvk_wanted_validation_layers[i]);
			if (!vecstr_push_back(kqvk_validation_layers_vec, &kqvk_wanted_validation_layers[i])) {
				KQ_OOM_MSG();
				free(avail_layers);
				vecstr_destroy(kqvk_validation_layers_vec);
				return false;
			}
		} else {
			LOG_MERROR("Missing validation layer %s.", kqvk_wanted_validation_layers[i]);
		}
	}
	free(avail_layers);

	// Don't free the vector; it's needed until the instance is created.
	instance_cinfo->enabledLayerCount   = (u32)kqvk_validation_layers_vec->size;
	instance_cinfo->ppEnabledLayerNames = kqvk_validation_layers_vec->p;

	return true;
}
#endif /* KQ_DEBUG */

static bool kqvk_instance_add_extensions(VkInstanceCreateInfo instance_cinfo[static 1]) {
	kqvk_instance_exts_vec = vecstr_create(0);
	if (!kqvk_instance_exts_vec) {
		KQ_OOM_MSG();
		return false;
	}

	u32          req_exts_count = 0;
	const char **exts           = glfwGetRequiredInstanceExtensions(&req_exts_count);

	for (u32 i = 0; i < req_exts_count; ++i) {
		LOG_MDEBUG("Enabling required instance extension %s.", exts[i]);
		if (!vecstr_push_back(kqvk_instance_exts_vec, &exts[i])) {
			KQ_OOM_MSG();
			vecstr_destroy(kqvk_instance_exts_vec);
			return false;
		}
	}

#if KQ_DEBUG
	LOG_MDEBUG("Enabling debug instance extension %s.", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	const char *dbg_ext = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	if (!vecstr_push_back(kqvk_instance_exts_vec, &dbg_ext)) {
		KQ_OOM_MSG();
		vecstr_destroy(kqvk_instance_exts_vec);
		return false;
	}
#endif /* KQ_DEBUG */

	instance_cinfo->enabledExtensionCount   = (u32)kqvk_instance_exts_vec->size;
	instance_cinfo->ppEnabledExtensionNames = kqvk_instance_exts_vec->p;

	return true;
}

#undef CB_LOG_MODULE
#define CB_LOG_MODULE "GLFW"
static void kq_callback_glfw_error(int e, const char *desc) {
	LOG_MERROR("GLFW (error %x): %s", e, desc);
}

#undef CB_LOG_MODULE
#define CB_LOG_MODULE "Vulkan"
#if KQ_DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL kq_callback_vk_debug(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                           VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                                           const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                           void                                       *pUserData) {
	CB_UNUSED(pUserData);
	CB_UNUSED(messageType);

	switch (messageSeverity) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		LOG_MERROR("%s.", pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		LOG_MWARN("%s.", pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		LOG_MDEBUG("%s.", pCallbackData->pMessage);
		break;
	default:
		break;
	}

	return VK_FALSE;
}
#endif /* KQ_DEBUG */
