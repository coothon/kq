#include "kq.h"

#include <GLFW/glfw3.h>

#include "libcbase/common.h"
#include "libcbase/log.h"
#include "libcbase/vec.h"
#define LIBCBASE_FS_IMPLEMENTATION
#include "libcbase/fs.h"

#define KQ_OOM_MSG() LOGM_FATAL("Out of memory! (OOM)")

#define CB_LOG_MODULE "KQ"

static void kq_callback_glfw_error(int e, const char *desc);

static int kqvk_reload_vulkan(VkInstance ins, VkPhysicalDevice pdev, VkDevice ldev);
#if KQ_DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL kq_callback_vk_debug(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                           VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                                           const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                           void                                       *pUserData);

static bool kqvk_instance_add_validation_layers(VkInstanceCreateInfo instance_cinfo[static 1]);
#endif /* KQ_DEBUG */
static bool kqvk_instance_add_extensions(VkInstanceCreateInfo instance_cinfo[static 1]);
static bool kqvk_choose_pdev(kq_data kq[static 1]);
static bool kqvk_check_pdev_for_extension(VkPhysicalDevice pdev, const char ext[restrict static 1]);
static bool kqvk_set_up_pdev_queues(kq_data kq[static 1]);
static void kqvk_ready_new_resolution(kq_data kq[static 1], int w, int h);
static bool kqvk_swapchain_create(kq_data kq[static 1]);
static bool kqvk_init_shaders(kq_data kq[static 1]);
static bool kqvk_render_pass_create(kq_data kq[static 1]);
static bool kqvk_pipeline_create(kq_data kq[static 1]);
static bool kqvk_framebuffers_create(kq_data kq[static 1]);
static bool kqvk_cmd_pool_and_buf_create(kq_data kq[static 1]);
static bool kqvk_vertex_buffer_create(kq_data kq[static 1]);
static bool kqvk_buffer_create(kq_data               kq[restrict static 1],
                               VkDeviceSize          size,
                               VkBufferUsageFlags    usage,
                               VkMemoryPropertyFlags props,
                               VkBuffer              buf[restrict static 1],
                               VkDeviceMemory        buf_mem[restrict static 1]);
static u32  kqvk_mem_type_find(kq_data kq[static 1], u32 type_filter, VkMemoryPropertyFlags props);
static void kqvk_buffer_copy(kq_data kq[static 1], VkBuffer src, VkBuffer dst, VkDeviceSize size);
static bool kqvk_sync_primitives_create(kq_data kq[static 1]);

cb_vec(vecstr, char *);

#if KQ_DEBUG
static const char *const kqvk_wanted_validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
static const u32         kqvk_wanted_validation_layers_count =
	sizeof kqvk_wanted_validation_layers / sizeof kqvk_wanted_validation_layers[0];
vecstr *kqvk_validation_layers_vec = 0;
#endif /* KQ_DEBUG */
vecstr *kqvk_instance_exts_vec = 0;

static const kq_vertex triangle_vertices[3] = {
	{{0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
	{ {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
	{{-1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
};


bool KQinit(kq_data kq[static 1]) {
	glfwSetErrorCallback(kq_callback_glfw_error);
	if (!glfwInit()) {
		LOGM_FATAL("GLFW initialization failed.");
		goto fail_glfwInit;
	}
	LOGM_TRACE("GLFW initialized.");

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
	glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	kq->win = glfwCreateWindow(800, 600, "kq", 0, 0);
	if (!kq->win) {
		LOGM_FATAL("GLFW window creation failed.");
		goto fail_glfwCreateWindow;
	}
	LOGM_TRACE("GLFW window created.");

	glfwSetWindowUserPointer(kq->win, kq);

	kq->vk_ver = kqvk_reload_vulkan(0, 0, 0);
	if (!kq->vk_ver)
		goto fail_glad_load_0_0_0;

#if KQ_DEBUG
	rend_info.debug_messenger_cinfo.pfnUserCallback = kq_callback_vk_debug;
#endif

#if KQ_DEBUG
	if (!kqvk_instance_add_validation_layers(&rend_info.instance_cinfo))
		goto fail_add_validation_layers;
#endif

	if (!kqvk_instance_add_extensions(&rend_info.instance_cinfo))
		goto fail_add_instance_extensions;

	if (vkCreateInstance(&rend_info.instance_cinfo, 0, &kq->vk_ins)) {
		LOGM_FATAL("Creating VkInstance failed.");
		goto fail_vkCreateInstance;
	}
	LOGM_TRACE("VkInstance created.");

	// Reload Vulkan after instance is created.
	kq->vk_ver = kqvk_reload_vulkan(kq->vk_ins, 0, 0);
	if (!kq->vk_ver)
		goto fail_glad_load_1_0_0;

#if KQ_DEBUG
	if (vkCreateDebugUtilsMessengerEXT(kq->vk_ins, &rend_info.debug_messenger_cinfo, 0, &kq->dbg_messenger)) {
		LOGM_FATAL("Unable to create debug messenger.");
		goto fail_vkCreateDebugUtilsMessengerEXT;
	}
	LOGM_DEBUG("Created debug messenger.");
#endif

	if (glfwCreateWindowSurface(kq->vk_ins, kq->win, 0, &kq->vk_surface)) {
		LOGM_FATAL("Unable to create Vulkan surface.");
		goto fail_glfwCreateWindowSurface;
	}
	LOGM_TRACE("VkSurfaceKHR created.");

	if (!kqvk_choose_pdev(kq))
		goto fail_choose_pdev;

	// Reload Vulkan after physical device is chosen.
	kq->vk_ver = kqvk_reload_vulkan(kq->vk_ins, kq->vk_pdev, 0);
	if (!kq->vk_ver)
		goto fail_glad_load_1_1_0;

	if (!kqvk_set_up_pdev_queues(kq))
		goto fail_set_up_pdev_queues;
	LOGM_TRACE("Pysical device queues chosen.");

	if (vkCreateDevice(kq->vk_pdev, &rend_info.ldevice_cinfo, 0, &kq->vk_ldev)) {
		LOGM_FATAL("Unable to create VkDevice.");
		goto fail_vkCreateDevice;
	}
	LOGM_TRACE("VkDevice created.");

	// Reload Vulkan for the final time, now that we have all we need.
	kq->vk_ver = kqvk_reload_vulkan(kq->vk_ins, kq->vk_pdev, kq->vk_ldev);
	if (!kq->vk_ver)
		goto fail_glad_load_1_1_1;

	// Get the queues.
	vkGetDeviceQueue(kq->vk_ldev, kq->q_graphics_index, 0, &kq->q_graphics);
	vkGetDeviceQueue(kq->vk_ldev, kq->q_present_index, 0, &kq->q_present);

	if (!kqvk_swapchain_create(kq)) {
		LOGM_FATAL("Unable to create swapchain.");
		goto fail_swapchain_create;
	}
	LOGM_TRACE("Swapchain created.");

	if (!kqvk_init_shaders(kq))
		goto fail_init_shaders;
	LOGM_TRACE("Shaders initialised.");

	if (!kqvk_render_pass_create(kq))
		goto fail_render_pass_create;

	if (!kqvk_pipeline_create(kq))
		goto fail_pipeline_create;

	if (!kqvk_framebuffers_create(kq))
		goto fail_framebuffers_create;

	if (!kqvk_cmd_pool_and_buf_create(kq))
		goto fail_cmd_pool_and_buf_create;

	if (!kqvk_sync_primitives_create(kq))
		goto fail_sync_primitives_create;

	return true;



fail_sync_primitives_create:
	vkDestroyBuffer(kq->vk_ldev, kq->vertex_buf, 0);
	vkFreeMemory(kq->vk_ldev, kq->vertex_buf_mem, 0);
	vkDestroyCommandPool(kq->vk_ldev, kq->cmd_pool, 0);
fail_cmd_pool_and_buf_create:
	for (u32 i = 0U; i < kq->swapchain_img_count; ++i)
		vkDestroyFramebuffer(kq->vk_ldev, kq->fbos[i], 0);
	free(kq->fbos);
fail_framebuffers_create:
	vkDestroyPipeline(kq->vk_ldev, kq->graphics_pipeline, 0);
	vkDestroyPipelineLayout(kq->vk_ldev, kq->pipeline_layout, 0);
fail_pipeline_create:
	vkDestroyRenderPass(kq->vk_ldev, kq->render_pass, 0);
fail_render_pass_create:
	vkDestroyShaderModule(kq->vk_ldev, kq->frag_module, 0);
	vkDestroyShaderModule(kq->vk_ldev, kq->vert_module, 0);
fail_init_shaders:
	for (u32 i = 0U; i < kq->swapchain_img_count; ++i)
		vkDestroyImageView(kq->vk_ldev, kq->swapchain_img_views[i], 0);
	free(kq->swapchain_img_views);
	free(kq->swapchain_imgs);
	vkDestroySwapchainKHR(kq->vk_ldev, kq->vk_swapchain, 0);
fail_swapchain_create:
fail_glad_load_1_1_1:
	vkDestroyDevice(kq->vk_ldev, 0);
fail_vkCreateDevice:
fail_set_up_pdev_queues:
fail_glad_load_1_1_0:
fail_choose_pdev:
	vkDestroySurfaceKHR(kq->vk_ins, kq->vk_surface, 0);
fail_glfwCreateWindowSurface:
#if KQ_DEBUG
	vkDestroyDebugUtilsMessengerEXT(kq->vk_ins, kq->dbg_messenger, 0);
fail_vkCreateDebugUtilsMessengerEXT:
#endif
fail_glad_load_1_0_0:
	vkDestroyInstance(kq->vk_ins, 0);
fail_vkCreateInstance:
	vecstr_destroy(kqvk_instance_exts_vec);
fail_add_instance_extensions:
#if KQ_DEBUG
	vecstr_destroy(kqvk_validation_layers_vec);
fail_add_validation_layers:
	gladUninstallVulkanDebug();
#endif
	gladLoaderUnloadVulkan();
fail_glad_load_0_0_0:
	glfwDestroyWindow(kq->win);
fail_glfwCreateWindow:
	glfwTerminate();
fail_glfwInit:
	return false;
}

void KQstop(kq_data kq[static 1]) {
	for (size_t i = 0; i < KQ_FRAMES_IN_FLIGHT; ++i) {
		vkDestroySemaphore(kq->vk_ldev, kq->render_finished_semaphore[i], 0);
		vkDestroySemaphore(kq->vk_ldev, kq->img_available_semaphore[i], 0);
		vkDestroyFence(kq->vk_ldev, kq->in_flight_fence[i], 0);
	}
	vkDestroyBuffer(kq->vk_ldev, kq->vertex_buf, 0);
	vkFreeMemory(kq->vk_ldev, kq->vertex_buf_mem, 0);
	vkDestroyCommandPool(kq->vk_ldev, kq->cmd_pool, 0);
	for (u32 i = 0U; i < kq->swapchain_img_count; ++i)
		vkDestroyFramebuffer(kq->vk_ldev, kq->fbos[i], 0);
	free(kq->fbos);
	vkDestroyPipeline(kq->vk_ldev, kq->graphics_pipeline, 0);
	vkDestroyPipelineLayout(kq->vk_ldev, kq->pipeline_layout, 0);
	vkDestroyRenderPass(kq->vk_ldev, kq->render_pass, 0);
	vkDestroyShaderModule(kq->vk_ldev, kq->frag_module, 0);
	vkDestroyShaderModule(kq->vk_ldev, kq->vert_module, 0);
	for (u32 i = 0U; i < kq->swapchain_img_count; ++i)
		vkDestroyImageView(kq->vk_ldev, kq->swapchain_img_views[i], 0);
	free(kq->swapchain_img_views);
	free(kq->swapchain_imgs);
	vkDestroySwapchainKHR(kq->vk_ldev, kq->vk_swapchain, 0);
	vkDestroyDevice(kq->vk_ldev, 0);
	vkDestroySurfaceKHR(kq->vk_ins, kq->vk_surface, 0);
#if KQ_DEBUG
	vkDestroyDebugUtilsMessengerEXT(kq->vk_ins, kq->dbg_messenger, 0);
#endif
	vkDestroyInstance(kq->vk_ins, 0);
	vecstr_destroy(kqvk_instance_exts_vec);
#if KQ_DEBUG
	vecstr_destroy(kqvk_validation_layers_vec);
#endif
	gladUninstallVulkanDebug();
	gladLoaderUnloadVulkan();
	glfwDestroyWindow(kq->win);
	glfwTerminate();
}



#undef CB_LOG_MODULE
#define CB_LOG_MODULE "KQVK"
static int kqvk_reload_vulkan(VkInstance ins, VkPhysicalDevice pdev, VkDevice ldev) {
	const register int vk_ver = gladLoaderLoadVulkan(ins, pdev, ldev);
	if (!vk_ver) {
		LOGM_FATAL("GLAD Vulkan loader failed.");
		return 0;
	}
	LOGM_DEBUG("Vulkan loaded (%s, %s, %s).",
	           ins ? "VkInstance" : "0",
	           pdev ? "VkPhysicalDevice" : "0",
	           ldev ? "VkDevice" : "0");
	LOGM_INFO("Vulkan %d.%d.", GLAD_VERSION_MAJOR(vk_ver), GLAD_VERSION_MINOR(vk_ver));
	gladInstallVulkanDebug();
	return vk_ver;
}

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
			LOGM_DEBUG("Enabling validation layer %s.", kqvk_wanted_validation_layers[i]);
			if (!vecstr_push_back(kqvk_validation_layers_vec, &kqvk_wanted_validation_layers[i])) {
				KQ_OOM_MSG();
				free(avail_layers);
				vecstr_destroy(kqvk_validation_layers_vec);
				return false;
			}
		} else {
			LOGM_ERROR("Missing validation layer %s.", kqvk_wanted_validation_layers[i]);
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
		LOGM_DEBUG("Enabling required instance extension %s.", exts[i]);
		if (!vecstr_push_back(kqvk_instance_exts_vec, &exts[i])) {
			KQ_OOM_MSG();
			vecstr_destroy(kqvk_instance_exts_vec);
			return false;
		}
	}

#if KQ_DEBUG
	LOGM_DEBUG("Enabling debug instance extension %s.", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
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

static bool kqvk_choose_pdev(kq_data kq[static 1]) {
	u32 pdev_count = 0U;
	vkEnumeratePhysicalDevices(kq->vk_ins, &pdev_count, 0);
	VkPhysicalDevice *pdevs = malloc(sizeof(VkPhysicalDevice[pdev_count]));
	if (!pdevs) {
		KQ_OOM_MSG();
		return false;
	}
	vkEnumeratePhysicalDevices(kq->vk_ins, &pdev_count, pdevs);

	u32 chosen_pdev = pdev_count;
	for (u32 i = 0U; i < pdev_count; ++i) {
		if (kqvk_check_pdev_for_extension(pdevs[i], VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
			chosen_pdev = i;
			break;
		}
	}

	if (chosen_pdev == pdev_count) {
		LOGM_FATAL("No suitable physical device.");
		free(pdevs);
		return false;
	}

	kq->vk_pdev = pdevs[chosen_pdev];
	free(pdevs);

	VkPhysicalDeviceProperties pdev_props = {0};
	vkGetPhysicalDeviceProperties(kq->vk_pdev, &pdev_props);
	LOGM_INFO("\"%s\" chosen as Vulkan physical device.", pdev_props.deviceName);

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(kq->vk_pdev, kq->vk_surface, &kq->vk_surface_capabilities);

	int w, h;
	glfwGetFramebufferSize(kq->win, &w, &h);
	kqvk_ready_new_resolution(kq, w, h);

	rend_info.swapchain_cinfo.preTransform = kq->vk_surface_capabilities.currentTransform;
	rend_info.swapchain_cinfo.surface      = kq->vk_surface;
	// Set to KQ_FRAMES_IN_FLIGHT + 1, assuming minImageCount is 2, which it probably is.
	rend_info.swapchain_cinfo.minImageCount = kq->vk_surface_capabilities.minImageCount + KQ_FRAMES_IN_FLIGHT - 1;
	if (kq->vk_surface_capabilities.maxImageCount > 0
	    && rend_info.swapchain_cinfo.minImageCount > kq->vk_surface_capabilities.maxImageCount)
		rend_info.swapchain_cinfo.minImageCount = kq->vk_surface_capabilities.maxImageCount;

	return true;
}

static bool kqvk_check_pdev_for_extension(VkPhysicalDevice pdev, const char ext[restrict static 1]) {
	u32 pdev_exts_count = 0U;
	vkEnumerateDeviceExtensionProperties(pdev, 0, &pdev_exts_count, 0);
	VkExtensionProperties *pdev_exts = malloc(sizeof(VkExtensionProperties[pdev_exts_count]));
	if (!pdev_exts) {
		KQ_OOM_MSG();
		return false;
	}
	vkEnumerateDeviceExtensionProperties(pdev, 0, &pdev_exts_count, pdev_exts);

	bool found = false;
	for (u32 i = 0U; i < pdev_exts_count; ++i) {
		if (!strcmp(ext, pdev_exts[i].extensionName)) {
			found = true;
			break;
		}
	}
	free(pdev_exts);

	return found;
}

static bool kqvk_set_up_pdev_queues(kq_data kq[static 1]) {
	u32 q_family_count = 0U;
	vkGetPhysicalDeviceQueueFamilyProperties(kq->vk_pdev, &q_family_count, 0);
	VkQueueFamilyProperties *q_families = malloc(sizeof(VkQueueFamilyProperties[q_family_count]));
	if (!q_families) {
		KQ_OOM_MSG();
		return false;
	}
	vkGetPhysicalDeviceQueueFamilyProperties(kq->vk_pdev, &q_family_count, q_families);

	bool g_found = false;
	bool p_found = false;
	for (u32 i = 0U; i < q_family_count; ++i) {
		if (!g_found && q_families[i].queueCount & VK_QUEUE_GRAPHICS_BIT) {
			kq->q_graphics_index = i;
			g_found              = true;
		}

		VkBool32 present_supported = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(kq->vk_pdev, i, kq->vk_surface, &present_supported);
		if (present_supported) {
			kq->q_present_index = i;
			p_found             = true;
		}

		if (g_found && p_found)
			break;
	}
	free(q_families);

	if (!g_found) {
		LOGM_FATAL("Chosen physical device does not have a graphics queue.");
		return false;
	}

	if (!p_found) {
		LOGM_FATAL("Chosen physical device does not have a present queue.");
		return false;
	}

	// Set them up as separate queues first.
	kq->q_priorities[0]                       = 1.0f;
	kq->q_priorities[1]                       = 1.0f;
	rend_info.q_cinfo[0].queueFamilyIndex     = kq->q_graphics_index;
	rend_info.q_cinfo[0].pQueuePriorities     = &kq->q_priorities[0];
	rend_info.q_cinfo[1].queueFamilyIndex     = kq->q_present_index;
	rend_info.q_cinfo[1].pQueuePriorities     = &kq->q_priorities[1];
	rend_info.cmd_pool_cinfo.queueFamilyIndex = kq->q_graphics_index;

	// If they are the same queue, we must not initialize them as separate.
	if (kq->q_graphics_index == kq->q_present_index) {
		rend_info.q_cinfo[0].queueCount              = 2;
		rend_info.ldevice_cinfo.queueCreateInfoCount = 1;
		rend_info.swapchain_cinfo.imageSharingMode   = VK_SHARING_MODE_EXCLUSIVE;
	} else {
		rend_info.swapchain_cinfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
		rend_info.swapchain_cinfo.queueFamilyIndexCount = 2;
		rend_info.swapchain_cinfo.pQueueFamilyIndices   = kq->q_indices_as_array;
	}

	return true;
}

// Does not recreate swapchain, but must be called before making a new one.
static void kqvk_ready_new_resolution(kq_data kq[static 1], int w, int h) {
	LOGM_TRACE("Setting resolution to (%d, %d).", w, h);
	kq->viewport.width                    = (float)w;
	kq->viewport.height                   = (float)h;
	rend_info.swapchain_cinfo.imageExtent = (VkExtent2D){(u32)w, (u32)h};
	if (rend_info.swapchain_cinfo.imageExtent.width < kq->vk_surface_capabilities.minImageExtent.width)
		rend_info.swapchain_cinfo.imageExtent.width = kq->vk_surface_capabilities.minImageExtent.width;
	else if (rend_info.swapchain_cinfo.imageExtent.width > kq->vk_surface_capabilities.maxImageExtent.width)
		rend_info.swapchain_cinfo.imageExtent.width = kq->vk_surface_capabilities.maxImageExtent.width;
	if (rend_info.swapchain_cinfo.imageExtent.height < kq->vk_surface_capabilities.minImageExtent.height)
		rend_info.swapchain_cinfo.imageExtent.height = kq->vk_surface_capabilities.minImageExtent.height;
	else if (rend_info.swapchain_cinfo.imageExtent.height > kq->vk_surface_capabilities.maxImageExtent.height)
		rend_info.swapchain_cinfo.imageExtent.height = kq->vk_surface_capabilities.maxImageExtent.height;
	rend_info.pass_begin_info.renderArea.extent = rend_info.swapchain_cinfo.imageExtent;
	kq->scissor.extent                          = rend_info.swapchain_cinfo.imageExtent;
	rend_info.fbo_cinfo.width                   = rend_info.swapchain_cinfo.imageExtent.width;
	rend_info.fbo_cinfo.height                  = rend_info.swapchain_cinfo.imageExtent.height;
	rend_info.pass_begin_info.renderArea.extent = rend_info.swapchain_cinfo.imageExtent;
}

static bool kqvk_swapchain_create(kq_data kq[static 1]) {
	if (vkCreateSwapchainKHR(kq->vk_ldev, &rend_info.swapchain_cinfo, 0, &kq->vk_swapchain)) {
		LOGM_FATAL("Unable to create swapchain.");
		return false;
	}

	vkGetSwapchainImagesKHR(kq->vk_ldev, kq->vk_swapchain, &kq->swapchain_img_count, 0);
	kq->swapchain_imgs = malloc(sizeof(VkImage[kq->swapchain_img_count]));
	if (!kq->swapchain_imgs) {
		KQ_OOM_MSG();
		vkDestroySwapchainKHR(kq->vk_ldev, kq->vk_swapchain, 0);
		return false;
	}
	vkGetSwapchainImagesKHR(kq->vk_ldev, kq->vk_swapchain, &kq->swapchain_img_count, kq->swapchain_imgs);

	kq->swapchain_img_views = malloc(sizeof(VkImageView[kq->swapchain_img_count]));
	if (!kq->swapchain_img_views) {
		KQ_OOM_MSG();
		free(kq->swapchain_imgs);
		vkDestroySwapchainKHR(kq->vk_ldev, kq->vk_swapchain, 0);
		return false;
	}

	for (u32 i = 0U; i < kq->swapchain_img_count; ++i) {
		rend_info.swapchain_img_view_cinfo.image = kq->swapchain_imgs[i];
		if (vkCreateImageView(kq->vk_ldev,
		                      &rend_info.swapchain_img_view_cinfo,
		                      0,
		                      &kq->swapchain_img_views[i])) {
			LOGM_FATAL("Unable to create swapchain image view %u.", i + 1);
			for (u32 j = 0U; j < i; ++j)
				vkDestroyImageView(kq->vk_ldev, kq->swapchain_img_views[j], 0);
			free(kq->swapchain_img_views);
			free(kq->swapchain_imgs);
			vkDestroySwapchainKHR(kq->vk_ldev, kq->vk_swapchain, 0);
			return false;
		}
	}

	rend_info.present_info.pSwapchains = &kq->vk_swapchain;

	return true;
}

static bool kqvk_init_shaders(kq_data kq[static 1]) {
	size_t vert_len = 0;
	u32   *vert_buf = fs_file_read_all_alloc("shaders/triangle.vert.spv", &vert_len);
	if (!(vert_buf && !(vert_len % 4))) { // codeSize must be a multiple of 4.
		KQ_OOM_MSG();
		return false;
	}

	size_t frag_len = 0;
	u32   *frag_buf = fs_file_read_all_alloc("shaders/triangle.frag.spv", &frag_len);
	if (!(frag_buf && !(frag_len % 4))) {
		KQ_OOM_MSG();
		return false;
	}

	rend_info.shader_module_vertex_cinfo.codeSize   = vert_len;
	rend_info.shader_module_fragment_cinfo.codeSize = frag_len;
	rend_info.shader_module_vertex_cinfo.pCode      = vert_buf;
	rend_info.shader_module_fragment_cinfo.pCode    = frag_buf;

	if (vkCreateShaderModule(kq->vk_ldev, &rend_info.shader_module_vertex_cinfo, 0, &kq->vert_module)) {
		LOGM_FATAL("Unable to create vertex shader module.");
		free(frag_buf);
		free(vert_buf);
		return false;
	}

	if (vkCreateShaderModule(kq->vk_ldev, &rend_info.shader_module_fragment_cinfo, 0, &kq->frag_module)) {
		LOGM_FATAL("Unable to create fragment shader module.");
		vkDestroyShaderModule(kq->vk_ldev, kq->vert_module, 0);
		free(frag_buf);
		free(vert_buf);
		return false;
	}

	rend_info.shader_stages_cinfo[0].module = kq->vert_module;
	rend_info.shader_stages_cinfo[1].module = kq->frag_module;

	return true;
}

static bool kqvk_render_pass_create(kq_data kq[static 1]) {
	kq->viewport.maxDepth                              = 1.0f;
	rend_info.pipeline_viewport_state_cinfo.pViewports = &kq->viewport;
	rend_info.pipeline_viewport_state_cinfo.pScissors  = &kq->scissor;

	if (vkCreateRenderPass(kq->vk_ldev, &rend_info.pass_cinfo, 0, &kq->render_pass)) {
		LOGM_FATAL("Unable to create vkRenderPass.");
		return false;
	}
	LOGM_TRACE("Created vkRenderPass.");

	rend_info.graphics_pipeline_cinfo.renderPass = kq->render_pass;
	rend_info.fbo_cinfo.renderPass               = kq->render_pass;
	rend_info.pass_begin_info.renderPass         = kq->render_pass;
	return true;
}

static bool kqvk_pipeline_create(kq_data kq[static 1]) {
	if (vkCreatePipelineLayout(kq->vk_ldev, &rend_info.pipeline_layout_cinfo, 0, &kq->pipeline_layout)) {
		LOGM_FATAL("Unable to create graphics pipeline layout.");
		return false;
	}

	rend_info.graphics_pipeline_cinfo.layout = kq->pipeline_layout;

	if (vkCreateGraphicsPipelines(kq->vk_ldev,
	                              0,
	                              1,
	                              &rend_info.graphics_pipeline_cinfo,
	                              0,
	                              &kq->graphics_pipeline)) {
		LOGM_FATAL("Unable to create graphics pipeline.");
		vkDestroyPipelineLayout(kq->vk_ldev, kq->pipeline_layout, 0);
		return false;
	}

	LOGM_TRACE("Graphics pipeline created.");
	return true;
}

static bool kqvk_framebuffers_create(kq_data kq[static 1]) {
	kq->fbos = malloc(sizeof(VkFramebuffer[kq->swapchain_img_count]));
	if (!kq->fbos) {
		KQ_OOM_MSG();
		return false;
	}

	for (u32 i = 0U; i < kq->swapchain_img_count; ++i) {
		rend_info.fbo_cinfo.pAttachments = &kq->swapchain_img_views[i];
		if (vkCreateFramebuffer(kq->vk_ldev, &rend_info.fbo_cinfo, 0, &kq->fbos[i])) {
			LOGM_FATAL("Unable to create framebuffer %u.", i);
			for (u32 j = 0U; j < i; ++j)
				vkDestroyFramebuffer(kq->vk_ldev, kq->fbos[j], 0);
			free(kq->fbos);
			return false;
		}
	}

	return true;
}

static bool kqvk_cmd_pool_and_buf_create(kq_data kq[static 1]) {
	if (vkCreateCommandPool(kq->vk_ldev, &rend_info.cmd_pool_cinfo, 0, &kq->cmd_pool)) {
		LOGM_FATAL("Unable to create command pool.");
		return false;
	}
	LOGM_TRACE("Command pool created.");

	rend_info.cmd_buf_allocate_info.commandPool = kq->cmd_pool;

	if (!kqvk_vertex_buffer_create(kq)) {
		vkDestroyCommandPool(kq->vk_ldev, kq->cmd_pool, 0);
		return false;
	}

	if (vkAllocateCommandBuffers(kq->vk_ldev, &rend_info.cmd_buf_allocate_info, kq->cmd_buf)) {
		LOGM_FATAL("Unable to create command buffer.");
		vkDestroyBuffer(kq->vk_ldev, kq->vertex_buf, 0);
		vkFreeMemory(kq->vk_ldev, kq->vertex_buf_mem, 0);
		vkDestroyCommandPool(kq->vk_ldev, kq->cmd_pool, 0);
		return false;
	}
	LOGM_TRACE("Command buffer created.");

	return true;
}

static bool kqvk_vertex_buffer_create(kq_data kq[static 1]) {
	VkBuffer       staging_buf;
	VkDeviceMemory staging_buf_mem;
	kqvk_buffer_create(kq,
	                   sizeof(kq_vertex) * 3,
	                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	                   &staging_buf,
	                   &staging_buf_mem);

	void *mapped_buf_mem;
	vkMapMemory(kq->vk_ldev, staging_buf_mem, 0, rend_info.vertex_buf_cinfo.size, 0, &mapped_buf_mem);
	memcpy(mapped_buf_mem, triangle_vertices, rend_info.vertex_buf_cinfo.size);
	vkUnmapMemory(kq->vk_ldev, staging_buf_mem);

	kqvk_buffer_create(kq, rend_info.vertex_buf_cinfo.size,
	                   VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &kq->vertex_buf, &kq->vertex_buf_mem);

	kqvk_buffer_copy(kq, staging_buf, kq->vertex_buf, rend_info.vertex_buf_cinfo.size);

	vkDestroyBuffer(kq->vk_ldev, staging_buf, 0);
	vkFreeMemory(kq->vk_ldev, staging_buf_mem, 0);

	return true;
}

static bool kqvk_buffer_create(kq_data               kq[restrict static 1],
                               VkDeviceSize          size,
                               VkBufferUsageFlags    usage,
                               VkMemoryPropertyFlags props,
                               VkBuffer              buf[restrict static 1],
                               VkDeviceMemory        buf_mem[restrict static 1]) {
	VkBufferCreateInfo buf_cinfo = {.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	                                .size        = size,
	                                .usage       = usage,
	                                .sharingMode = VK_SHARING_MODE_EXCLUSIVE};
	if (vkCreateBuffer(kq->vk_ldev, &buf_cinfo, 0, buf))
		return false;

	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(kq->vk_ldev, *buf, &mem_reqs);

	VkMemoryAllocateInfo mem_ainfo = {.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
	                                  .allocationSize  = mem_reqs.size,
	                                  .memoryTypeIndex = kqvk_mem_type_find(kq, mem_reqs.memoryTypeBits, props)};
	if (vkAllocateMemory(kq->vk_ldev, &mem_ainfo, 0, buf_mem)) {
		vkDestroyBuffer(kq->vk_ldev, *buf, 0);
		return false;
	}

	vkBindBufferMemory(kq->vk_ldev, *buf, *buf_mem, 0);
	return true;
}

static void kqvk_buffer_copy(kq_data kq[static 1], VkBuffer src, VkBuffer dst, VkDeviceSize size) {
	VkCommandBufferAllocateInfo cmd_buf_ainfo = {.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	                                             .level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	                                             .commandPool = kq->cmd_pool,
	                                             .commandBufferCount = 1};
	VkCommandBuffer             cmd_buf;

	vkAllocateCommandBuffers(kq->vk_ldev, &cmd_buf_ainfo, &cmd_buf);

	VkCommandBufferBeginInfo cmd_buf_binfo = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	                                          .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

	vkBeginCommandBuffer(cmd_buf, &cmd_buf_binfo);
	VkBufferCopy copy_region = {.size = size};
	vkCmdCopyBuffer(cmd_buf, src, dst, 1, &copy_region);
	vkEndCommandBuffer(cmd_buf);

	VkSubmitInfo sinfo = {.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
	                      .commandBufferCount = 1,
	                      .pCommandBuffers    = &cmd_buf};

	vkQueueSubmit(kq->q_graphics, 1, &sinfo, 0);
	vkQueueWaitIdle(kq->q_graphics);
	vkFreeCommandBuffers(kq->vk_ldev, kq->cmd_pool, 1, &cmd_buf);
}

static u32 kqvk_mem_type_find(kq_data kq[static 1], u32 type_filter, VkMemoryPropertyFlags props) {
	VkPhysicalDeviceMemoryProperties mem_props;
	vkGetPhysicalDeviceMemoryProperties(kq->vk_pdev, &mem_props);

	for (u32 i = 0U; i < mem_props.memoryTypeCount; ++i) {
		if ((type_filter & (1 << i)) && ((mem_props.memoryTypes[i].propertyFlags & props) == props))
			return i;
	}

	LOGM_ERROR("No suitable device memory types.");
	return 0;
}

static bool kqvk_sync_primitives_create(kq_data kq[static 1]) {
	for (size_t i = 0; i < KQ_FRAMES_IN_FLIGHT; ++i) {
		if (vkCreateFence(kq->vk_ldev, &rend_info.fence_cinfo, 0, &kq->in_flight_fence[i])) {
			for (size_t j = 0; j < i; ++j) {
				vkDestroySemaphore(kq->vk_ldev, kq->render_finished_semaphore[j], 0);
				vkDestroySemaphore(kq->vk_ldev, kq->img_available_semaphore[j], 0);
				vkDestroyFence(kq->vk_ldev, kq->in_flight_fence[j], 0);
			}
			return false;
		}
		if (vkCreateSemaphore(kq->vk_ldev, &rend_info.semaphore_cinfo, 0, &kq->img_available_semaphore[i])) {
			vkDestroyFence(kq->vk_ldev, kq->in_flight_fence[i], 0);
			for (size_t j = 0; j < i; ++j) {
				vkDestroySemaphore(kq->vk_ldev, kq->render_finished_semaphore[j], 0);
				vkDestroySemaphore(kq->vk_ldev, kq->img_available_semaphore[j], 0);
				vkDestroyFence(kq->vk_ldev, kq->in_flight_fence[j], 0);
			}
			return false;
		}
		if (vkCreateSemaphore(kq->vk_ldev, &rend_info.semaphore_cinfo, 0, &kq->render_finished_semaphore[i])) {
			vkDestroyFence(kq->vk_ldev, kq->in_flight_fence[i], 0);
			vkDestroySemaphore(kq->vk_ldev, kq->img_available_semaphore[i], 0);
			for (size_t j = 0; j < i; ++j) {
				vkDestroySemaphore(kq->vk_ldev, kq->render_finished_semaphore[j], 0);
				vkDestroySemaphore(kq->vk_ldev, kq->img_available_semaphore[j], 0);
				vkDestroyFence(kq->vk_ldev, kq->in_flight_fence[j], 0);
			}
			return false;
		}
	}

	LOGM_TRACE("Created synchronisation primitives.");
	return true;
}


#undef CB_LOG_MODULE
#define CB_LOG_MODULE "GLFW"
static void kq_callback_glfw_error(int e, const char *desc) {
	LOGM_ERROR("GLFW (error %x): %s", e, desc);
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
		LOGM_ERROR("%s.", pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		LOGM_WARN("%s.", pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		LOGM_DEBUG("%s.", pCallbackData->pMessage);
		break;
	default:
		break;
	}

	return VK_FALSE;
}
#endif /* KQ_DEBUG */
