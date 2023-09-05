#ifndef KQ_H_
#define KQ_H_

#include <stdbool.h>

#include "glad/vulkan.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

#include "libcbase/common.h"

#define KQ_FRAMES_IN_FLIGHT 2

typedef struct kq_data {
	GLFWwindow *win;
	bool        fb_resized;

	// Vulkan.
	VkInstance               vk_ins;
	VkSurfaceKHR             vk_surface;
	VkPhysicalDevice         vk_pdev;
	VkDevice                 vk_ldev;
	VkSurfaceCapabilitiesKHR vk_surface_capabilities;

	// Swapchain.
	VkSwapchainKHR vk_swapchain;
	u32            swapchain_img_count;
	VkImage       *swapchain_imgs;
	VkImageView   *swapchain_img_views;
	VkFramebuffer *fbos;

	// Queues.
	VkQueue q_graphics;
	VkQueue q_present;
	float   q_priorities[2];
	union {
		u32 q_indices_as_array[2];
		struct {
			u32 q_graphics_index;
			u32 q_present_index;
		};
	};

	// Rendering setup.
	VkShaderModule   vert_module;
	VkShaderModule   frag_module;
	VkViewport       viewport;
	VkRect2D         scissor;
	VkRenderPass     render_pass;
	VkPipelineLayout pipeline_layout;
	VkPipeline       graphics_pipeline;
	VkCommandPool    cmd_pool;
	VkCommandBuffer  cmd_buf[KQ_FRAMES_IN_FLIGHT];
	VkBuffer         vertex_buf;
	VkDeviceMemory   vertex_buf_mem;

	// Synchronization primitives.
	VkSemaphore img_available_semaphore[KQ_FRAMES_IN_FLIGHT];
	VkSemaphore render_finished_semaphore[KQ_FRAMES_IN_FLIGHT];
	VkFence     in_flight_fence[KQ_FRAMES_IN_FLIGHT];

#if KQ_DEBUG
	VkDebugUtilsMessengerEXT dbg_messenger;
#endif

	int vk_ver;
} kq_data;

typedef struct kq_info {
	VkApplicationInfo                      app_info;
	VkInstanceCreateInfo                   instance_cinfo;
	VkDeviceQueueCreateInfo                q_cinfo[2];
	VkDeviceCreateInfo                     ldevice_cinfo;
	VkSwapchainCreateInfoKHR               swapchain_cinfo;
	VkImageViewCreateInfo                  swapchain_img_view_cinfo;
	VkShaderModuleCreateInfo               shader_module_vertex_cinfo;
	VkShaderModuleCreateInfo               shader_module_fragment_cinfo;
	VkPipelineShaderStageCreateInfo        shader_stages_cinfo[2];
	VkDynamicState                         pipeline_dynamic_states[2];
	VkPipelineDynamicStateCreateInfo       pipeline_dynamic_states_cinfo;
	VkPipelineVertexInputStateCreateInfo   vertex_input_state_cinfo;
	VkPipelineInputAssemblyStateCreateInfo pipeline_assembly_input_state_cinfo;
	VkPipelineViewportStateCreateInfo      pipeline_viewport_state_cinfo;
	VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_cinfo;
	VkPipelineMultisampleStateCreateInfo   pipeline_msaa_state_cinfo;
	VkPipelineColorBlendAttachmentState    pipeline_color_blend_attachment_state;
	VkPipelineColorBlendStateCreateInfo    pipeline_color_blend_cinfo;
	VkPipelineLayoutCreateInfo             pipeline_layout_cinfo;
	VkAttachmentDescription                pass_color_attachment;
	VkAttachmentReference                  pass_color_attachment_ref;
	VkSubpassDescription                   subpass_desc;
	VkRenderPassCreateInfo                 pass_cinfo;
	VkGraphicsPipelineCreateInfo           graphics_pipeline_cinfo;
	VkFramebufferCreateInfo                fbo_cinfo;
#if KQ_DEBUG
	VkDebugUtilsMessengerCreateInfoEXT debug_messenger_cinfo;
#endif
	VkCommandPoolCreateInfo           cmd_pool_cinfo;
	VkCommandBufferAllocateInfo       cmd_buf_allocate_info;
	VkCommandBufferBeginInfo          cmd_buf_begin_info;
	VkClearValue                      clear_color;
	VkRenderPassBeginInfo             pass_begin_info;
	VkSemaphoreCreateInfo             semaphore_cinfo;
	VkFenceCreateInfo                 fence_cinfo;
	VkSubmitInfo                      submit_info;
	VkPipelineStageFlagBits           submit_dst_stage_mask;
	VkSubpassDependency               subpass_dep;
	VkPresentInfoKHR                  present_info;
	VkVertexInputBindingDescription   vertex_input_binding_desc;
	VkVertexInputAttributeDescription vertex_input_attrib_descs[2];
	VkBufferCreateInfo                vertex_buf_cinfo;
} kq_info;

typedef struct kq_vertex {
	vec2 position;
	vec3 color;
} kq_vertex;

extern kq_info rend_info;

extern bool KQinit(kq_data kq[static 1]);
extern void KQstop(kq_data kq[static 1]);

#endif /* KQ_H_ */
