#include <kqvk.h>

#include <stdlib.h>
#include <tgmath.h>

#include <glad/vulkan.h>
#include <kq.h>
#include <libcbase/log.h>
#include <libcbase/fs.h>


#define CB_LOG_MODULE "KQVK"


#if KQ_DEBUG
static const char *const kqvk_wanted_validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
static const u32         kqvk_wanted_validation_layers_count = sizeof kqvk_wanted_validation_layers / sizeof kqvk_wanted_validation_layers[0];
vecstr                  *kqvk_validation_layers_vec = 0;
#endif /* KQ_DEBUG */
vecstr *kqvk_instance_exts_vec = 0;


#if KQ_DEBUG
bool kqvk_instance_add_validation_layers(VkInstanceCreateInfo instance_cinfo[static 1]) {
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
	instance_cinfo->enabledLayerCount = (u32)kqvk_validation_layers_vec->size;
	instance_cinfo->ppEnabledLayerNames = kqvk_validation_layers_vec->p;

	return true;
}
#endif /* KQ_DEBUG */

bool kqvk_instance_add_extensions(VkInstanceCreateInfo instance_cinfo[static 1]) {
	kqvk_instance_exts_vec = vecstr_create(0);
	if (!kqvk_instance_exts_vec) {
		KQ_OOM_MSG();
		return false;
	}

	u32          req_exts_count = 0;
	const char **exts = glfwGetRequiredInstanceExtensions(&req_exts_count);

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

	instance_cinfo->enabledExtensionCount = (u32)kqvk_instance_exts_vec->size;
	instance_cinfo->ppEnabledExtensionNames = kqvk_instance_exts_vec->p;

	return true;
}

bool kqvk_check_pdev_for_extension(VkPhysicalDevice pdev, const char ext[restrict static 1]) {
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

bool kqvk_choose_pdev(kq_data kq[static 1]) {
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
	// Set to KQ_FRAMES_IN_FLIGHT + 1, assuming minImageCount is 2, which it probably is.
	rend_info.swapchain_cinfo.minImageCount = kq->vk_surface_capabilities.minImageCount + KQ_FRAMES_IN_FLIGHT - 1;
	if (kq->vk_surface_capabilities.maxImageCount > 0 && rend_info.swapchain_cinfo.minImageCount > kq->vk_surface_capabilities.maxImageCount)
		rend_info.swapchain_cinfo.minImageCount = kq->vk_surface_capabilities.maxImageCount;

	return true;
}

int kqvk_reload_vulkan(VkInstance ins, VkPhysicalDevice pdev, VkDevice ldev) {
	const register int vk_ver = gladLoaderLoadVulkan(ins, pdev, ldev);
	if (!vk_ver) {
		LOGM_FATAL("GLAD Vulkan loader failed.");
		return 0;
	}
	LOGM_DEBUG("Vulkan %sloaded (%s, %s, %s).",
	           (ins || pdev || ldev) ? "re" : "",
	           ins ? "VkInstance" : "NULL",
	           pdev ? "VkPhysicalDevice" : "NULL",
	           ldev ? "VkDevice" : "NULL");
	if (ins && pdev && ldev)
		LOGM_INFO("Vulkan %d.%d.", GLAD_VERSION_MAJOR(vk_ver), GLAD_VERSION_MINOR(vk_ver));
	gladInstallVulkanDebug();
	return vk_ver;
}

bool kqvk_set_up_pdev_queues(kq_data kq[static 1]) {
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
		if (!g_found && q_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			kq->q_graphics_index = i;
			g_found = true;
		}

		VkBool32 present_supported = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(kq->vk_pdev, i, kq->vk_surface, &present_supported);
		if (present_supported) {
			kq->q_present_index = i;
			p_found = true;
		}

		if (g_found && p_found)
			break;
	}
	free(q_families);

	if (!g_found) {
		LOGM_FATAL("Chosen physical device does not have a graphics "
		           "queue.");
		return false;
	}

	if (!p_found) {
		LOGM_FATAL("Chosen physical device does not have a present "
		           "queue.");
		return false;
	}

	// Set them up as separate queues first.
	kq->q_priorities[0] = 1.0f;
	kq->q_priorities[1] = 1.0f;
	rend_info.q_cinfo[0].queueFamilyIndex = kq->q_graphics_index;
	rend_info.q_cinfo[0].pQueuePriorities = &kq->q_priorities[0];
	rend_info.q_cinfo[1].queueFamilyIndex = kq->q_present_index;
	rend_info.q_cinfo[1].pQueuePriorities = &kq->q_priorities[1];
	rend_info.cmd_pool_cinfo.queueFamilyIndex = kq->q_graphics_index;

	// If they are the same queue, we must not initialize them as separate.
	if (kq->q_graphics_index == kq->q_present_index) {
		rend_info.q_cinfo[0].queueCount = 2;
		rend_info.ldevice_cinfo.queueCreateInfoCount = 1;
		rend_info.swapchain_cinfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	} else {
		rend_info.swapchain_cinfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		rend_info.swapchain_cinfo.queueFamilyIndexCount = 2;
		rend_info.swapchain_cinfo.pQueueFamilyIndices = kq->q_indices_as_array;
	}

	return true;
}

void kqvk_ready_new_resolution(kq_data kq[static 1], int w, int h) {
	LOGM_TRACE("Setting resolution to (%d, %d).", w, h);
	kq->viewport.width = (float)w;
	kq->viewport.height = (float)h;
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
	kq->scissor.extent = rend_info.swapchain_cinfo.imageExtent;
	rend_info.fbo_cinfo.width = rend_info.swapchain_cinfo.imageExtent.width;
	rend_info.fbo_cinfo.height = rend_info.swapchain_cinfo.imageExtent.height;
	rend_info.pass_begin_info.renderArea.extent = rend_info.swapchain_cinfo.imageExtent;
}

u32 kqvk_mem_type_find(kq_data kq[static 1], u32 type_filter, VkMemoryPropertyFlags props) {
	VkPhysicalDeviceMemoryProperties mem_props;
	vkGetPhysicalDeviceMemoryProperties(kq->vk_pdev, &mem_props);

	for (u32 i = 0U; i < mem_props.memoryTypeCount; ++i) {
		if ((type_filter & (1 << i)) && ((mem_props.memoryTypes[i].propertyFlags & props) == props))
			return i;
	}

	LOGM_ERROR("No suitable device memory types.");
	return 0;
}

VkCommandBuffer kqvk_single_time_command_begin(kq_data kq[static 1]) {
	VkCommandBufferAllocateInfo ainfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandPool = kq->cmd_pool,
		.commandBufferCount = 1,
	};

	VkCommandBuffer cmd_buf;
	vkAllocateCommandBuffers(kq->vk_ldev, &ainfo, &cmd_buf);

	VkCommandBufferBeginInfo binfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};

	vkBeginCommandBuffer(cmd_buf, &binfo);

	return cmd_buf;
}

void kqvk_single_time_command_end(kq_data kq[restrict static 1], VkCommandBuffer cmd_buf) {
	vkEndCommandBuffer(cmd_buf);

	VkSubmitInfo sinfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &cmd_buf,
	};

	vkQueueSubmit(kq->q_graphics, 1, &sinfo, 0);
	vkQueueWaitIdle(kq->q_graphics);

	vkFreeCommandBuffers(kq->vk_ldev, kq->cmd_pool, 1, &cmd_buf);
}

bool kqvk_buffer_create(kq_data               kq[restrict static 1],
                        VkDeviceSize          size,
                        VkBufferUsageFlags    usage,
                        VkMemoryPropertyFlags props,
                        VkBuffer              buf[restrict static 1],
                        VkDeviceMemory        buf_mem[restrict static 1]) {
	VkBufferCreateInfo buf_cinfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = size, .usage = usage, .sharingMode = VK_SHARING_MODE_EXCLUSIVE};
	if (vkCreateBuffer(kq->vk_ldev, &buf_cinfo, 0, buf))
		return false;

	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(kq->vk_ldev, *buf, &mem_reqs);

	VkMemoryAllocateInfo mem_ainfo = {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
	                                  .allocationSize = mem_reqs.size,
	                                  .memoryTypeIndex = kqvk_mem_type_find(kq, mem_reqs.memoryTypeBits, props)};
	if (vkAllocateMemory(kq->vk_ldev, &mem_ainfo, 0, buf_mem)) {
		vkDestroyBuffer(kq->vk_ldev, *buf, 0);
		return false;
	}

	vkBindBufferMemory(kq->vk_ldev, *buf, *buf_mem, 0);
	return true;
}

void kqvk_buffer_copy(kq_data kq[static 1], VkBuffer src, VkBuffer dst, VkDeviceSize size) {
	VkCommandBuffer cmd_buf = kqvk_single_time_command_begin(kq);

	VkBufferCopy copy_region = {.size = size};
	vkCmdCopyBuffer(cmd_buf, src, dst, 1, &copy_region);

	kqvk_single_time_command_end(kq, cmd_buf);
}

bool kqvk_image_create(kq_data               kq[restrict static 1],
                       u32                   tw,
                       u32                   th,
                       u32                   array_layers,
                       VkFormat              fmt,
                       VkImageTiling         tiling,
                       VkImageUsageFlags     usage,
                       VkMemoryPropertyFlags props,
                       VkImage               img[restrict static 1],
                       VkDeviceMemory        img_mem[restrict static 1]) {
	VkImageCreateInfo image_cinfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.extent = (VkExtent3D){.width = tw, .height = th, .depth = 1},
		.mipLevels = 1,
		.arrayLayers = array_layers,
		.format = fmt,
		.tiling = tiling,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.usage = usage,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};

	if (vkCreateImage(kq->vk_ldev, &image_cinfo, 0, img)) {
		return false;
	}

	VkMemoryRequirements mem_reqs;
	vkGetImageMemoryRequirements(kq->vk_ldev, *img, &mem_reqs);

	VkMemoryAllocateInfo ainfo = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = mem_reqs.size,
		.memoryTypeIndex = kqvk_mem_type_find(kq, mem_reqs.memoryTypeBits, props),
	};

	if (vkAllocateMemory(kq->vk_ldev, &ainfo, 0, img_mem)) {
		vkDestroyImage(kq->vk_ldev, *img, 0);
		return false;
	}

	vkBindImageMemory(kq->vk_ldev, *img, *img_mem, 0);

	return true;
}

bool kqvk_image_layout_transition(kq_data       kq[restrict static 1],
                                  VkImage       img,
                                  u32           array_layers,
                                  VkFormat      fmt,
                                  VkImageLayout old_layout,
                                  VkImageLayout new_layout) {
	CB_UNUSED(fmt);

	VkImageMemoryBarrier barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.oldLayout = old_layout,
		.newLayout = new_layout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = img,
		.subresourceRange =
			(VkImageSubresourceRange){
						  .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
						  .levelCount = 1,
						  .layerCount = array_layers,
						  },
	};

	VkPipelineStageFlags src_stage, dst_stage;
	if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else {
		LOGM_FATAL("Image layout transition not supported.");
		return false;
	}

	VkCommandBuffer cmd_buf = kqvk_single_time_command_begin(kq);
	vkCmdPipelineBarrier(cmd_buf, src_stage, dst_stage, 0, 0, 0, 0, 0, 1, &barrier);
	kqvk_single_time_command_end(kq, cmd_buf);

	return true;
}

void kqvk_buffer_copy_to_image(kq_data kq[restrict static 1], VkBuffer buf, VkImage img, u32 width, u32 height, u32 array_layers) {
	VkCommandBuffer cmd_buf = kqvk_single_time_command_begin(kq);

	VkBufferImageCopy region = {
		.imageSubresource = (VkImageSubresourceLayers){.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .layerCount = array_layers},
		.imageExtent = (VkExtent3D){.width = width, .height = height, .depth = 1},
	};

	vkCmdCopyBufferToImage(cmd_buf, buf, img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	kqvk_single_time_command_end(kq, cmd_buf);
}

stbi_uc *kqvk_tex_load(const char path[restrict static 1], int desired_width, int desired_height, int desired_channels) {
	int      width, height, channels;
	stbi_uc *pix = stbi_load(path, &width, &height, &channels, desired_channels);
	if (!pix) {
		LOGM_ERROR("%s: %s.", stbi_failure_reason(), path);
		return 0;
	}

	if (width != desired_width && height != desired_height) {
		LOGM_ERROR("%s: Loaded texture size does not match desired texture size. Wanted %dx%d, but got %dx%d.",
		           path,
		           desired_width,
		           desired_height,
		           width,
		           height);
		stbi_image_free(pix);
		return 0;
	}

	LOGM_TRACE("Loaded image \"%s\" as texture.", path);
	return pix;
}



bool kqvk_create_swapchain(kq_data kq[static 1]) {
	rend_info.swapchain_cinfo.oldSwapchain = kq->vk_swapchain;
	if (vkCreateSwapchainKHR(kq->vk_ldev, &rend_info.swapchain_cinfo, 0, &kq->vk_swapchain)) {
		LOGM_FATAL("Unable to create swapchain.");
		vkDestroySwapchainKHR(kq->vk_ldev, rend_info.swapchain_cinfo.oldSwapchain, 0);
		return false;
	}
	vkDestroySwapchainKHR(kq->vk_ldev, rend_info.swapchain_cinfo.oldSwapchain, 0);

	u32 tmp_count;
	vkGetSwapchainImagesKHR(kq->vk_ldev, kq->vk_swapchain, &tmp_count, 0);
	if (!kq->swapchain_imgs || kq->swapchain_img_count != tmp_count) { // First call or img_count is different from last time.
		if (kq->swapchain_imgs)
			free(kq->swapchain_imgs);
		if (kq->swapchain_img_views)
			free(kq->swapchain_img_views);
		if (kq->fbos) {
			free(kq->fbos);
			kq->fbos = 0;
		}
		kq->swapchain_img_count = tmp_count;

		kq->swapchain_imgs = malloc(sizeof(VkImage[kq->swapchain_img_count]));
		if (!kq->swapchain_imgs) {
			KQ_OOM_MSG();
			vkDestroySwapchainKHR(kq->vk_ldev, kq->vk_swapchain, 0);
			return false;
		}

		kq->swapchain_img_views = malloc(sizeof(VkImageView[kq->swapchain_img_count]));
		if (!kq->swapchain_img_views) {
			KQ_OOM_MSG();
			free(kq->swapchain_imgs);
			vkDestroySwapchainKHR(kq->vk_ldev, kq->vk_swapchain, 0);
			return false;
		}
	}

	vkGetSwapchainImagesKHR(kq->vk_ldev, kq->vk_swapchain, &kq->swapchain_img_count, kq->swapchain_imgs);

	for (u32 i = 0U; i < kq->swapchain_img_count; ++i) {
		rend_info.swapchain_img_view_cinfo.image = kq->swapchain_imgs[i];
		if (vkCreateImageView(kq->vk_ldev, &rend_info.swapchain_img_view_cinfo, 0, &kq->swapchain_img_views[i])) {
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

bool kqvk_init_shaders(kq_data kq[static 1]) {
	size_t tiles_vert_len = 0;
	u32   *tiles_vert_buf = fs_file_read_all_alloc("shaders/tile.vert.spv", &tiles_vert_len);
	if (!(tiles_vert_buf && !(tiles_vert_len % 4))) { // codeSize must be a multiple of 4.
		KQ_OOM_MSG();
		return false;
	}

	size_t tiles_frag_len = 0;
	u32   *tiles_frag_buf = fs_file_read_all_alloc("shaders/tile.frag.spv", &tiles_frag_len);
	if (!(tiles_frag_buf && !(tiles_frag_len % 4))) {
		KQ_OOM_MSG();
		return false;
	}

	rend_info.tiles_shader_module_vertex_cinfo.codeSize = tiles_vert_len;
	rend_info.tiles_shader_module_fragment_cinfo.codeSize = tiles_frag_len;
	rend_info.tiles_shader_module_vertex_cinfo.pCode = tiles_vert_buf;
	rend_info.tiles_shader_module_fragment_cinfo.pCode = tiles_frag_buf;

	if (vkCreateShaderModule(kq->vk_ldev, &rend_info.tiles_shader_module_vertex_cinfo, 0, &kq->tiles_vert_module)) {
		LOGM_FATAL("Unable to create vertex shader module.");
		free(tiles_frag_buf);
		free(tiles_vert_buf);
		return false;
	}
	free(tiles_vert_buf);

	if (vkCreateShaderModule(kq->vk_ldev, &rend_info.tiles_shader_module_fragment_cinfo, 0, &kq->tiles_frag_module)) {
		LOGM_FATAL("Unable to create fragment shader module.");
		vkDestroyShaderModule(kq->vk_ldev, kq->tiles_vert_module, 0);
		free(tiles_frag_buf);
		return false;
	}
	free(tiles_frag_buf);

	rend_info.tiles_shader_stages_cinfo[0].module = kq->tiles_vert_module;
	rend_info.tiles_shader_stages_cinfo[1].module = kq->tiles_frag_module;

	return true;
}

bool kqvk_create_render_pass(kq_data kq[static 1]) {
	kq->viewport.maxDepth = 1.0f;
	rend_info.pipeline_viewport_state_cinfo.pViewports = &kq->viewport;
	rend_info.pipeline_viewport_state_cinfo.pScissors = &kq->scissor;

	if (vkCreateRenderPass(kq->vk_ldev, &rend_info.pass_cinfo, 0, &kq->render_pass)) {
		LOGM_FATAL("Unable to create vkRenderPass.");
		return false;
	}
	LOGM_TRACE("Created vkRenderPass.");

	rend_info.graphics_pipeline_cinfo.renderPass = kq->render_pass;
	rend_info.fbo_cinfo.renderPass = kq->render_pass;
	rend_info.pass_begin_info.renderPass = kq->render_pass;
	return true;
}

bool kqvk_create_descriptor_set_layout(kq_data kq[static 1]) {
	if (vkCreateDescriptorSetLayout(kq->vk_ldev, &rend_info.descriptor_set_layout_cinfo, 0, &kq->descriptor_set_layout)) {
		LOGM_FATAL("Unable to create descriptor set layout.");
		return false;
	}

	return true;
}

bool kqvk_create_pipeline(kq_data kq[static 1]) {
	rend_info.pipeline_layout_cinfo.pSetLayouts = &kq->descriptor_set_layout;

	if (vkCreatePipelineLayout(kq->vk_ldev, &rend_info.pipeline_layout_cinfo, 0, &kq->pipeline_layout)) {
		LOGM_FATAL("Unable to create graphics pipeline layout.");
		return false;
	}

	rend_info.graphics_pipeline_cinfo.layout = kq->pipeline_layout;

	if (vkCreateGraphicsPipelines(kq->vk_ldev, 0, 1, &rend_info.graphics_pipeline_cinfo, 0, &kq->graphics_pipeline)) {
		LOGM_FATAL("Unable to create graphics pipeline.");
		vkDestroyPipelineLayout(kq->vk_ldev, kq->pipeline_layout, 0);
		return false;
	}

	LOGM_TRACE("Graphics pipeline created.");
	return true;
}

bool kqvk_create_framebuffers(kq_data kq[static 1]) {
	if (!kq->fbos) { // In case this is not the first call.
		kq->fbos = malloc(sizeof(VkFramebuffer[kq->swapchain_img_count]));
		if (!kq->fbos) {
			KQ_OOM_MSG();
			return false;
		}
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

bool kqvk_create_cmd_pool(kq_data kq[static 1]) {
	if (vkCreateCommandPool(kq->vk_ldev, &rend_info.cmd_pool_cinfo, 0, &kq->cmd_pool)) {
		LOGM_FATAL("Unable to create command pool.");
		return false;
	}
	LOGM_TRACE("Command pool created.");

	rend_info.cmd_buf_allocate_info.commandPool = kq->cmd_pool;

	return true;
}

bool kqvk_create_cmd_bufs(kq_data kq[static 1]) {
	if (vkAllocateCommandBuffers(kq->vk_ldev, &rend_info.cmd_buf_allocate_info, kq->cmd_buf)) {
		LOGM_FATAL("Unable to create command buffer.");
		return false;
	}
	LOGM_TRACE("Command buffer created.");

	return true;
}

bool kqvk_create_vertex_buffer(kq_data kq[static 1]) {
	VkBuffer              staging_buf;
	VkDeviceMemory        staging_buf_mem;
	register const size_t buf_size = sizeof(kq_vertex[KQ_QUAD_NUM_VERTICES]);

	kqvk_buffer_create(kq,
	                   buf_size,
	                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	                   &staging_buf,
	                   &staging_buf_mem);

	void *mapped_buf_mem;
	vkMapMemory(kq->vk_ldev, staging_buf_mem, 0, buf_size, 0, &mapped_buf_mem);
	memcpy(mapped_buf_mem, quad_vertices, buf_size);
	vkUnmapMemory(kq->vk_ldev, staging_buf_mem);

	kqvk_buffer_create(kq,
	                   buf_size,
	                   VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	                   &kq->vertex_buf,
	                   &kq->vertex_buf_mem);

	kqvk_buffer_copy(kq, staging_buf, kq->vertex_buf, buf_size);

	vkDestroyBuffer(kq->vk_ldev, staging_buf, 0);
	vkFreeMemory(kq->vk_ldev, staging_buf_mem, 0);

	return true;
}

bool kqvk_create_index_buffer(kq_data kq[static 1]) {
	VkBuffer              staging_buf;
	VkDeviceMemory        staging_buf_mem;
	register const size_t buf_size = sizeof(u16[KQ_QUAD_NUM_INDICES]);

	kqvk_buffer_create(kq,
	                   buf_size,
	                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	                   &staging_buf,
	                   &staging_buf_mem);

	void *mapped_buf_mem;
	vkMapMemory(kq->vk_ldev, staging_buf_mem, 0, buf_size, 0, &mapped_buf_mem);
	memcpy(mapped_buf_mem, quad_indices, buf_size);
	vkUnmapMemory(kq->vk_ldev, staging_buf_mem);

	kqvk_buffer_create(kq,
	                   buf_size,
	                   VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	                   &kq->index_buf,
	                   &kq->index_buf_mem);

	kqvk_buffer_copy(kq, staging_buf, kq->index_buf, buf_size);

	vkDestroyBuffer(kq->vk_ldev, staging_buf, 0);
	vkFreeMemory(kq->vk_ldev, staging_buf_mem, 0);

	return true;
}

bool kqvk_create_tiles_tex(kq_data kq[restrict static 1]) {
	stbi_uc *img1 = kqvk_tex_load("textures/tiles/1.png", KQ_TILES_IMAGE_WIDTH, KQ_TILES_IMAGE_HEIGHT, STBI_rgb_alpha);
	if (!img1)
		return 0;

	stbi_uc *img2 = kqvk_tex_load("textures/tiles/2.png", KQ_TILES_IMAGE_WIDTH, KQ_TILES_IMAGE_HEIGHT, STBI_rgb_alpha);
	if (!img2) {
		stbi_image_free(img1);
		return 0;
	}

	VkBuffer       stage_buf;
	VkDeviceMemory stage_mem;
	kqvk_buffer_create(kq,
	                   KQ_TILES_IMAGE_SIZE * KQ_TILES_IMAGE_COUNT,
	                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	                   &stage_buf,
	                   &stage_mem);

	void *data;
	vkMapMemory(kq->vk_ldev, stage_mem, 0, KQ_TILES_IMAGE_SIZE * KQ_TILES_IMAGE_COUNT, 0, &data);
	memcpy(data, img1, KQ_TILES_IMAGE_SIZE);
	memcpy((uchar *)data + KQ_TILES_IMAGE_SIZE, img2, KQ_TILES_IMAGE_SIZE);
	vkUnmapMemory(kq->vk_ldev, stage_mem);

	stbi_image_free(img1);
	stbi_image_free(img2);

	kqvk_image_create(kq,
	                  KQ_TILES_IMAGE_WIDTH,
	                  KQ_TILES_IMAGE_HEIGHT,
	                  KQ_TILES_IMAGE_COUNT,
	                  VK_FORMAT_R8G8B8A8_SRGB,
	                  VK_IMAGE_TILING_OPTIMAL,
	                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	                  &kq->tiles_tex_image,
	                  &kq->tiles_tex_mem);

	if (!kqvk_image_layout_transition(kq,
	                                  kq->tiles_tex_image,
	                                  KQ_TILES_IMAGE_COUNT,
	                                  VK_FORMAT_R8G8B8A8_SRGB,
	                                  VK_IMAGE_LAYOUT_UNDEFINED,
	                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)) {
		vkDestroyBuffer(kq->vk_ldev, stage_buf, 0);
		vkFreeMemory(kq->vk_ldev, stage_mem, 0);
		vkDestroyImage(kq->vk_ldev, kq->tiles_tex_image, 0);
		vkFreeMemory(kq->vk_ldev, kq->tiles_tex_mem, 0);
		return false;
	}

	kqvk_buffer_copy_to_image(kq, stage_buf, kq->tiles_tex_image, KQ_TILES_IMAGE_WIDTH, KQ_TILES_IMAGE_HEIGHT, KQ_TILES_IMAGE_COUNT);

	if (!kqvk_image_layout_transition(kq,
	                                  kq->tiles_tex_image,
	                                  KQ_TILES_IMAGE_COUNT,
	                                  VK_FORMAT_R8G8B8A8_SRGB,
	                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)) {
		vkDestroyBuffer(kq->vk_ldev, stage_buf, 0);
		vkFreeMemory(kq->vk_ldev, stage_mem, 0);
		vkDestroyImage(kq->vk_ldev, kq->tiles_tex_image, 0);
		vkFreeMemory(kq->vk_ldev, kq->tiles_tex_mem, 0);
		return false;
	}

	vkDestroyBuffer(kq->vk_ldev, stage_buf, 0);
	vkFreeMemory(kq->vk_ldev, stage_mem, 0);

	LOGM_TRACE("Created texture.");
	return true;
}

bool kqvk_create_tiles_tex_view(kq_data kq[static 1]) {
	rend_info.tiles_tex_view_cinfo.image = kq->tiles_tex_image;

	if (vkCreateImageView(kq->vk_ldev, &rend_info.tiles_tex_view_cinfo, 0, &kq->tiles_tex_view)) {
		LOGM_FATAL("Failed to create texture image view.");
		return false;
	}

	rend_info.sampler_write.imageView = kq->tiles_tex_view;

	return true;
}

bool kqvk_create_tiles_tex_sampler(kq_data kq[static 1]) {
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(kq->vk_pdev, &props);

	rend_info.tiles_tex_sampler_cinfo.maxAnisotropy = props.limits.maxSamplerAnisotropy;

	if (vkCreateSampler(kq->vk_ldev, &rend_info.tiles_tex_sampler_cinfo, 0, &kq->tiles_tex_sampler)) {
		LOGM_FATAL("Failed to create texture sampler.");
		return false;
	}

	rend_info.sampler_write.sampler = kq->tiles_tex_sampler;

	return true;
}

bool kqvk_uniforms_init(kq_data kq[static 1]) {
	if (!kqvk_create_uniform_buffers(kq))
		return false;

	if (vkCreateDescriptorPool(kq->vk_ldev, &rend_info.desc_pool_cinfo, 0, &kq->desc_pool)) {
		for (size_t i = 0; i < KQ_FRAMES_IN_FLIGHT; ++i) {
			vkUnmapMemory(kq->vk_ldev, kq->uniform_bufs_mem[i]);
			vkDestroyBuffer(kq->vk_ldev, kq->uniform_bufs[i], 0);
			vkFreeMemory(kq->vk_ldev, kq->uniform_bufs_mem[i], 0);
		}

		return false;
	}

	if (!kqvk_create_descriptor_sets(kq)) {
		vkDestroyDescriptorPool(kq->vk_ldev, kq->desc_pool, 0);
		for (size_t i = 0; i < KQ_FRAMES_IN_FLIGHT; ++i) {
			vkUnmapMemory(kq->vk_ldev, kq->uniform_bufs_mem[i]);
			vkDestroyBuffer(kq->vk_ldev, kq->uniform_bufs[i], 0);
			vkFreeMemory(kq->vk_ldev, kq->uniform_bufs_mem[i], 0);
		}

		return false;
	}

	return true;
}

bool kqvk_create_sync_primitives(kq_data kq[static 1]) {
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



bool kqvk_create_uniform_buffers(kq_data kq[static 1]) {
	register const size_t buf_size = sizeof(kq_uniforms);

	for (size_t i = 0; i < KQ_FRAMES_IN_FLIGHT; ++i) {
		kqvk_buffer_create(kq,
		                   buf_size,
		                   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		                   &kq->uniform_bufs[i],
		                   &kq->uniform_bufs_mem[i]);

		vkMapMemory(kq->vk_ldev, kq->uniform_bufs_mem[i], 0, buf_size, 0, &kq->uniform_bufs_mapped[i]);
	}

	return true;
}

bool kqvk_swapchain_recreate(kq_data kq[static 1]) {
	vkDeviceWaitIdle(kq->vk_ldev);

	// Checks for minimized window.
	while (!(kq->scissor.extent.width | kq->scissor.extent.height))
		glfwWaitEvents();

	LOGM_TRACE("Recreating swapchain.");

	// Destroy old swapchain.
	for (u32 i = 0; i < kq->swapchain_img_count; ++i) {
		vkDestroyFramebuffer(kq->vk_ldev, kq->fbos[i], 0);
		vkDestroyImageView(kq->vk_ldev, kq->swapchain_img_views[i], 0);
	}
	//vkDestroySwapchainKHR(kq->vk_ldev, kq->vk_swapchain, 0);

	if (!kqvk_create_swapchain(kq))
		return false;
	if (!kqvk_create_framebuffers(kq))
		return false;

	return true;
}

void kqvk_uniforms_update_time(kq_data kq[static 1]) {
	register const double now = glfwGetTime();

	kq->uniforms.time = (float)now;
	kq->uniforms.time_sin = (float)(sin(now));
	kq->uniforms.time_cos = (float)(cos(now));
}

void kqvk_uniforms_push(kq_data kq[static 1]) {
	memcpy(kq->uniform_bufs_mapped[kq->current_frame], &kq->uniforms, sizeof(kq_uniforms));
}

bool kqvk_create_descriptor_sets(kq_data kq[static 1]) {
	VkDescriptorSetLayout layouts[KQ_FRAMES_IN_FLIGHT];
	for (size_t i = 0; i < KQ_FRAMES_IN_FLIGHT; ++i)
		layouts[i] = kq->descriptor_set_layout;

	rend_info.desc_sets_ainfo.descriptorPool = kq->desc_pool;
	rend_info.desc_sets_ainfo.pSetLayouts = layouts;

	if (vkAllocateDescriptorSets(kq->vk_ldev, &rend_info.desc_sets_ainfo, kq->desc_sets))
		return false;

	for (size_t i = 0; i < KQ_FRAMES_IN_FLIGHT; ++i) {
		rend_info.desc_binfo.buffer = kq->uniform_bufs[i];
		rend_info.desc_write[0].dstSet = kq->desc_sets[i];
		rend_info.desc_write[1].dstSet = kq->desc_sets[i];
		vkUpdateDescriptorSets(kq->vk_ldev, 2, rend_info.desc_write, 0, 0);
	}

	return true;
}
