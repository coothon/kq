#include <kq.h>
#include <kqvk.h>

#include <GLFW/glfw3.h>

#include <libcbase/common.h>
#include <libcbase/log.h>
#include <libcbase/vec.h>


#define CB_LOG_MODULE "KQ"


static void kq_callback_glfw_error(int e, const char *desc);
static void kq_callback_glfw_fb_resize(GLFWwindow *win, int w, int h);


#if KQ_DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL kq_callback_vk_debug(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                           VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                                           const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                           void                                       *pUserData);
#endif


const u16       quad_indices[6] = {0, 1, 2, 2, 3, 0};
const kq_vertex quad_vertices[4] = {
	{{-1.0f, -1.0f}, {0.0f, 0.0f}},
	{ {1.0f, -1.0f}, {1.0f, 0.0f}},
	{  {1.0f, 1.0f}, {1.0f, 1.0f}},
	{ {-1.0f, 1.0f}, {0.0f, 1.0f}},
};


bool KQinit(kq_data kq[static 1]) {
	glfwSetErrorCallback(kq_callback_glfw_error);

	// TODO: Don't force this.
	glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
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
	kq->win = glfwCreateWindow(800, 600, "kq", 0, 0);
	if (!kq->win) {
		LOGM_FATAL("GLFW window creation failed.");
		goto fail_glfwCreateWindow;
	}
	LOGM_TRACE("GLFW window created.");

	glfwSetWindowUserPointer(kq->win, kq);
	glfwSetFramebufferSizeCallback(kq->win, kq_callback_glfw_fb_resize);

	kq->vk_ver = kqvk_reload_vulkan(0, 0, 0);
	if (!kq->vk_ver)
		goto fail_glad_load_0_0_0;

#if KQ_DEBUG
	rend_info.debug_messenger_cinfo.pfnUserCallback = kq_callback_vk_debug;
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
	rend_info.swapchain_cinfo.surface = kq->vk_surface;
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

	if (!kqvk_create_swapchain(kq)) {
		LOGM_FATAL("Unable to create swapchain.");
		goto fail_create_swapchain;
	}
	LOGM_TRACE("Swapchain created.");

	if (!kqvk_init_shaders(kq))
		goto fail_init_shaders;
	LOGM_TRACE("Shaders initialised.");

	if (!kqvk_create_render_pass(kq))
		goto fail_create_render_pass;

	if (!kqvk_create_descriptor_set_layout(kq))
		goto fail_create_descriptor_set_layout;

	if (!kqvk_create_pipeline(kq))
		goto fail_create_pipeline;

	if (!kqvk_create_framebuffers(kq))
		goto fail_create_framebuffers;

	if (!kqvk_create_cmd_pool(kq))
		goto fail_create_cmd_pool;

	if (!kqvk_create_cmd_bufs(kq))
		goto fail_create_cmd_bufs;

	if (!kqvk_create_vertex_buffer(kq))
		goto fail_create_vertex_buffer;

	if (!kqvk_create_index_buffer(kq))
		goto fail_create_index_buffer;

	if (!kqvk_create_tiles_tex(kq))
		goto fail_create_tiles_tex;

	if (!kqvk_create_tiles_tex_view(kq))
		goto fail_create_tiles_tex_view;

	if (!kqvk_create_tiles_tex_sampler(kq))
		goto fail_create_tiles_tex_sampler;

	if (!kqvk_uniforms_init(kq))
		goto fail_uniforms_init;

	if (!kqvk_create_sync_primitives(kq))
		goto fail_create_sync_primitives;

	LOGM_INFO("Initialized.");
	return true;

fail_create_sync_primitives:
	vkDestroyDescriptorPool(kq->vk_ldev, kq->desc_pool, 0);
	for (size_t i = 0; i < KQ_FRAMES_IN_FLIGHT; ++i) {
		vkUnmapMemory(kq->vk_ldev, kq->uniform_bufs_mem[i]);
		vkDestroyBuffer(kq->vk_ldev, kq->uniform_bufs[i], 0);
		vkFreeMemory(kq->vk_ldev, kq->uniform_bufs_mem[i], 0);
	}
fail_uniforms_init:
	vkDestroySampler(kq->vk_ldev, kq->tiles_tex_sampler, 0);
fail_create_tiles_tex_sampler:
	vkDestroyImageView(kq->vk_ldev, kq->tiles_tex_view, 0);
fail_create_tiles_tex_view:
	vkDestroyImage(kq->vk_ldev, kq->tiles_tex_image, 0);
	vkFreeMemory(kq->vk_ldev, kq->tiles_tex_mem, 0);
fail_create_tiles_tex:
	vkDestroyBuffer(kq->vk_ldev, kq->index_buf, 0);
	vkFreeMemory(kq->vk_ldev, kq->index_buf_mem, 0);
fail_create_index_buffer:
	vkDestroyBuffer(kq->vk_ldev, kq->vertex_buf, 0);
	vkFreeMemory(kq->vk_ldev, kq->vertex_buf_mem, 0);
fail_create_vertex_buffer:
fail_create_cmd_bufs:
	vkDestroyCommandPool(kq->vk_ldev, kq->cmd_pool, 0);
fail_create_cmd_pool:
	for (u32 i = 0U; i < kq->swapchain_img_count; ++i)
		vkDestroyFramebuffer(kq->vk_ldev, kq->fbos[i], 0);
	free(kq->fbos);
fail_create_framebuffers:
	vkDestroyPipeline(kq->vk_ldev, kq->graphics_pipeline, 0);
	vkDestroyPipelineLayout(kq->vk_ldev, kq->pipeline_layout, 0);
fail_create_pipeline:
	vkDestroyDescriptorSetLayout(kq->vk_ldev, kq->descriptor_set_layout, 0);
fail_create_descriptor_set_layout:
	vkDestroyRenderPass(kq->vk_ldev, kq->render_pass, 0);
fail_create_render_pass:
	vkDestroyShaderModule(kq->vk_ldev, kq->tiles_frag_module, 0);
	vkDestroyShaderModule(kq->vk_ldev, kq->tiles_vert_module, 0);
fail_init_shaders:
	for (u32 i = 0U; i < kq->swapchain_img_count; ++i)
		vkDestroyImageView(kq->vk_ldev, kq->swapchain_img_views[i], 0);
	free(kq->swapchain_img_views);
	free(kq->swapchain_imgs);
	vkDestroySwapchainKHR(kq->vk_ldev, kq->vk_swapchain, 0);
fail_create_swapchain:
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
	LOGM_INFO("Stopping.");
	vkDeviceWaitIdle(kq->vk_ldev);

	for (size_t i = 0; i < KQ_FRAMES_IN_FLIGHT; ++i) {
		vkDestroySemaphore(kq->vk_ldev, kq->render_finished_semaphore[i], 0);
		vkDestroySemaphore(kq->vk_ldev, kq->img_available_semaphore[i], 0);
		vkDestroyFence(kq->vk_ldev, kq->in_flight_fence[i], 0);
	}
	vkDestroyDescriptorPool(kq->vk_ldev, kq->desc_pool, 0);
	for (size_t i = 0; i < KQ_FRAMES_IN_FLIGHT; ++i) {
		vkUnmapMemory(kq->vk_ldev, kq->uniform_bufs_mem[i]);
		vkDestroyBuffer(kq->vk_ldev, kq->uniform_bufs[i], 0);
		vkFreeMemory(kq->vk_ldev, kq->uniform_bufs_mem[i], 0);
	}
	vkDestroySampler(kq->vk_ldev, kq->tiles_tex_sampler, 0);
	vkDestroyImageView(kq->vk_ldev, kq->tiles_tex_view, 0);
	vkDestroyImage(kq->vk_ldev, kq->tiles_tex_image, 0);
	vkFreeMemory(kq->vk_ldev, kq->tiles_tex_mem, 0);
	vkDestroyBuffer(kq->vk_ldev, kq->index_buf, 0);
	vkFreeMemory(kq->vk_ldev, kq->index_buf_mem, 0);
	vkDestroyBuffer(kq->vk_ldev, kq->vertex_buf, 0);
	vkFreeMemory(kq->vk_ldev, kq->vertex_buf_mem, 0);
	vkDestroyCommandPool(kq->vk_ldev, kq->cmd_pool, 0);
	for (u32 i = 0U; i < kq->swapchain_img_count; ++i)
		vkDestroyFramebuffer(kq->vk_ldev, kq->fbos[i], 0);
	free(kq->fbos);
	vkDestroyPipeline(kq->vk_ldev, kq->graphics_pipeline, 0);
	vkDestroyPipelineLayout(kq->vk_ldev, kq->pipeline_layout, 0);
	vkDestroyDescriptorSetLayout(kq->vk_ldev, kq->descriptor_set_layout, 0);
	vkDestroyRenderPass(kq->vk_ldev, kq->render_pass, 0);
	vkDestroyShaderModule(kq->vk_ldev, kq->tiles_frag_module, 0);
	vkDestroyShaderModule(kq->vk_ldev, kq->tiles_vert_module, 0);
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

bool KQrender_begin(kq_data kq[static 1]) {
	if (kq->rendering)
		return false;

	// Wait for previous frame (of the same index) to finish.
	vkWaitForFences(kq->vk_ldev, 1, &kq->in_flight_fence[kq->current_frame], VK_TRUE, UINT64_MAX);

	if (kq->fb_resized) {
		if (!kqvk_swapchain_recreate(kq))
			return false;
		kq->fb_resized = false;
	}

retry_acquire:
	switch (vkAcquireNextImageKHR(kq->vk_ldev, kq->vk_swapchain, UINT64_MAX, kq->img_available_semaphore[kq->current_frame], 0, &kq->img_index)) {
	case VK_ERROR_OUT_OF_DATE_KHR:
		if (!kqvk_swapchain_recreate(kq))
			return false;
		goto retry_acquire;
	case VK_SUBOPTIMAL_KHR:
	case VK_SUCCESS:
		break;
	default:
		return false;
	}

	vkResetFences(kq->vk_ldev, 1, &kq->in_flight_fence[kq->current_frame]);
	vkResetCommandBuffer(kq->cmd_buf[kq->current_frame], 0);
	rend_info.submit_info.pCommandBuffers = &kq->cmd_buf[kq->current_frame];

	rend_info.submit_info.pWaitSemaphores = &kq->img_available_semaphore[kq->current_frame];
	rend_info.submit_info.pSignalSemaphores = &kq->render_finished_semaphore[kq->current_frame];
	rend_info.pass_begin_info.framebuffer = kq->fbos[kq->img_index];

	kqvk_uniforms_update_time(kq);
	kqvk_uniforms_push(kq);

	if (vkBeginCommandBuffer(kq->cmd_buf[kq->current_frame], &rend_info.cmd_buf_begin_info))
		return false;

	vkCmdBeginRenderPass(kq->cmd_buf[kq->current_frame], &rend_info.pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(kq->cmd_buf[kq->current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, kq->graphics_pipeline);
	vkCmdSetViewport(kq->cmd_buf[kq->current_frame], 0, 1, &kq->viewport);
	vkCmdSetScissor(kq->cmd_buf[kq->current_frame], 0, 1, &kq->scissor);

	VkDeviceSize vertex_buf_offset = 0;
	vkCmdBindVertexBuffers(kq->cmd_buf[kq->current_frame], 0, 1, &kq->vertex_buf, &vertex_buf_offset);
	vkCmdBindIndexBuffer(kq->cmd_buf[kq->current_frame], kq->index_buf, 0, VK_INDEX_TYPE_UINT16);
	vkCmdBindDescriptorSets(kq->cmd_buf[kq->current_frame],
	                        VK_PIPELINE_BIND_POINT_GRAPHICS,
	                        kq->pipeline_layout,
	                        0,
	                        1,
	                        &kq->desc_sets[kq->current_frame],
	                        0,
	                        0);


	kq->rendering = true;
	return true;
}

bool KQrender_end(kq_data kq[static 1]) {
	if (!kq->rendering)
		return false;

	vkCmdEndRenderPass(kq->cmd_buf[kq->current_frame]);

	if (vkEndCommandBuffer(kq->cmd_buf[kq->current_frame]))
		return false;

	if (vkQueueSubmit(kq->q_graphics, 1, &rend_info.submit_info, kq->in_flight_fence[kq->current_frame]))
		return false;

	rend_info.present_info.pImageIndices = &kq->img_index;
	rend_info.present_info.pWaitSemaphores = &kq->render_finished_semaphore[kq->current_frame];

	switch (vkQueuePresentKHR(kq->q_present, &rend_info.present_info)) {
	case VK_SUBOPTIMAL_KHR:
	case VK_ERROR_OUT_OF_DATE_KHR:
		if (!kqvk_swapchain_recreate(kq))
			return false;
	case VK_SUCCESS:
		break;
	default:
		return false;
	}

	kq->current_frame = (kq->current_frame + 1) % KQ_FRAMES_IN_FLIGHT;

	kq->rendering = false;
	return true;
}

bool KQdraw_quad(kq_data kq[static 1], const float pos[restrict static 2], const float scale[restrict static 2], u32 tiles_tex_index) {
	if (!kq->rendering)
		return false;

	kq->tiles_pcs.position[0] = pos[0];
	kq->tiles_pcs.position[1] = pos[1];
	kq->tiles_pcs.scale[0] = scale[0];
	kq->tiles_pcs.scale[1] = scale[1];
	kq->tiles_pcs.tiles_tex_index = (float)tiles_tex_index;

	vkCmdPushConstants(kq->cmd_buf[kq->current_frame],
	                   kq->pipeline_layout,
	                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
	                   0,
	                   sizeof(kq_tiles_pcs),
	                   &kq->tiles_pcs);
	vkCmdDrawIndexed(kq->cmd_buf[kq->current_frame], KQ_QUAD_NUM_INDICES, 1, 0, 0, 0);
	return true;
}


#undef CB_LOG_MODULE
#define CB_LOG_MODULE "GLFW"

static void kq_callback_glfw_error(int e, const char *desc) {
	LOGM_ERROR("GLFW (error %x): %s", e, desc);
}

static void kq_callback_glfw_fb_resize(GLFWwindow *win, int w, int h) {
	kq_data *kq = (kq_data *)glfwGetWindowUserPointer(win);
	if (kq) {
		kq->fb_resized = true;
		kqvk_ready_new_resolution(kq, w, h);
	}
}


#if KQ_DEBUG
	#undef CB_LOG_MODULE
	#define CB_LOG_MODULE "Vulkan"

static VKAPI_ATTR VkBool32 VKAPI_CALL kq_callback_vk_debug(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                           VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                                           const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                           void                                       *pUserData) {
	CB_UNUSED(pUserData);
	CB_UNUSED(messageType);

	switch (messageSeverity) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		LOGM_ERROR("%s", pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		LOGM_WARN("%s", pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		LOGM_DEBUG("%s", pCallbackData->pMessage);
		break;
	default:
		break;
	}

	return VK_FALSE;
}
#endif /* KQ_DEBUG */


cb_impl_vec(vecstr, char *);
