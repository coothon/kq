#ifndef KQ_H_
#define KQ_H_

#include <stdbool.h>
#include <stdalign.h>

#include <glad/vulkan.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <libcbase/common.h>
#include <libcbase/vec.h>


#define KQ_OOM_MSG() LOGM_FATAL("Out of memory! (OOM)")

#define KQ_FRAMES_IN_FLIGHT 2

#define KQ_TILES_IMAGE_COUNT  2
#define KQ_TILES_IMAGE_WIDTH  64
#define KQ_TILES_IMAGE_HEIGHT 64
#define KQ_TILES_IMAGE_SIZE   (KQ_TILES_IMAGE_WIDTH * KQ_TILES_IMAGE_HEIGHT * 4)

#define KQ_TILES_VERTEX_INPUT_ATTRIBUTES_NUM 2

#define KQ_QUAD_NUM_VERTICES 4
#define KQ_QUAD_NUM_INDICES  6

// Constants for the entire frame.
typedef struct kq_uniforms {
	alignas(4) float time;
	alignas(4) float time_sin;
	alignas(4) float time_cos;
} kq_uniforms;

// Push constants for tiles.
typedef struct kq_tiles_pcs {
	alignas(8) vec2 position;
	alignas(8) vec2 scale;
	alignas(4) float tiles_tex_index; // Texture arrays index with floats, for some ungodly reason.
} kq_tiles_pcs;

typedef struct kq_data {
	bool   rendering;
	size_t current_frame;
	u32    img_index;

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
	union { // I think this is UB. . . .
		u32 q_indices_as_array[2];
		struct {
			u32 q_graphics_index;
			u32 q_present_index;
		};
	};

	// Rendering setup.
	VkViewport            viewport;
	VkRect2D              scissor;
	VkRenderPass          render_pass;
	VkDescriptorSetLayout descriptor_set_layout;
	VkPipelineLayout      pipeline_layout;
	VkDescriptorPool      desc_pool;
	VkDescriptorSet       desc_sets[KQ_FRAMES_IN_FLIGHT];
	VkPipeline            graphics_pipeline;
	VkCommandPool         cmd_pool;
	VkCommandBuffer       cmd_buf[KQ_FRAMES_IN_FLIGHT];

	VkBuffer       vertex_buf;
	VkDeviceMemory vertex_buf_mem;
	VkBuffer       index_buf;
	VkDeviceMemory index_buf_mem;
	VkBuffer       uniform_bufs[KQ_FRAMES_IN_FLIGHT];
	VkDeviceMemory uniform_bufs_mem[KQ_FRAMES_IN_FLIGHT];
	void          *uniform_bufs_mapped[KQ_FRAMES_IN_FLIGHT];

	kq_uniforms uniforms;

	// Tiles.
	VkShaderModule tiles_vert_module;
	VkShaderModule tiles_frag_module;
	VkImage        tiles_tex_image;
	VkDeviceMemory tiles_tex_mem;
	VkImageView    tiles_tex_view;
	VkSampler      tiles_tex_sampler;
	kq_tiles_pcs   tiles_pcs;

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
	VkShaderModuleCreateInfo               tiles_shader_module_vertex_cinfo;
	VkShaderModuleCreateInfo               tiles_shader_module_fragment_cinfo;
	VkPipelineShaderStageCreateInfo        tiles_shader_stages_cinfo[2];
	VkDynamicState                         pipeline_dynamic_states[2];
	VkPipelineDynamicStateCreateInfo       pipeline_dynamic_states_cinfo;
	VkPipelineVertexInputStateCreateInfo   tiles_vertex_input_state_cinfo;
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
	VkVertexInputBindingDescription   tiles_vertex_input_binding_desc;
	VkVertexInputAttributeDescription tiles_vertex_input_attrib_descs[KQ_TILES_VERTEX_INPUT_ATTRIBUTES_NUM];
	union {
		VkDescriptorSetLayoutBinding layout_bindings[2];
		struct {
			VkDescriptorSetLayoutBinding ubo_layout_binding;
			VkDescriptorSetLayoutBinding sampler_layout_binding;
		};
	};
	VkDescriptorSetLayoutCreateInfo descriptor_set_layout_cinfo;
	VkDescriptorPoolSize            desc_pool_size[2];
	VkDescriptorPoolCreateInfo      desc_pool_cinfo;
	VkDescriptorSetAllocateInfo     desc_sets_ainfo;
	VkDescriptorBufferInfo          desc_binfo;
	VkWriteDescriptorSet            desc_write[2];
	VkDescriptorImageInfo           sampler_write;
	VkPhysicalDeviceFeatures        pdev_feats;
	VkPushConstantRange             tiles_pc_range;
	VkImageCreateInfo               tiles_tex_image_cinfo;
	VkImageViewCreateInfo           tiles_tex_view_cinfo;
	VkSamplerCreateInfo             tiles_tex_sampler_cinfo;
} kq_info;

typedef struct kq_vertex {
	vec2 position;
	vec2 uv;
} kq_vertex;

cb_mk_vec(vecstr, char *);


extern kq_info         rend_info;
extern const u16       quad_indices[KQ_QUAD_NUM_INDICES];
extern const kq_vertex quad_vertices[KQ_QUAD_NUM_VERTICES];


extern bool KQinit(kq_data kq[static 1]);

extern void KQstop(kq_data kq[static 1]);


extern bool KQrender_begin(kq_data kq[static 1]);

extern bool KQrender_end(kq_data kq[static 1]);

extern bool KQdraw_quad(kq_data kq[static 1], const float pos[restrict static 2], const float scale[restrict static 2], u32 tiles_tex_index);

#endif /* KQ_H_ */
