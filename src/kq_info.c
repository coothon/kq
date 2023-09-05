#include "kq.h"

// All that can be specified at compile time.
kq_info rend_info = {
	.app_info =
		(VkApplicationInfo){.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                    .pApplicationName = "kq",
                                    .applicationVersion =
                                            VK_MAKE_VERSION(0, 0, 1),
                                    .pEngineName   = "kq",
                                    .engineVersion = VK_MAKE_VERSION(0, 0, 1),
                                    .apiVersion    = VK_API_VERSION_1_2},
	.instance_cinfo =
		(VkInstanceCreateInfo){
				    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
#if KQ_DEBUG
				    .pNext = &rend_info.debug_messenger_cinfo,
#endif
				    .pApplicationInfo = &rend_info.app_info},
	.q_cinfo = {(VkDeviceQueueCreateInfo){.sType =
                                                      VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                              .queueCount = 1},
                                    (VkDeviceQueueCreateInfo){.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                              .queueCount = 1}},
	.ldevice_cinfo =
		(VkDeviceCreateInfo){
				    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
				    .queueCreateInfoCount  = 2,
				    .pQueueCreateInfos     = rend_info.q_cinfo,
				    .enabledExtensionCount = 1,
				    .ppEnabledExtensionNames =
				(const char *[]){
					VK_KHR_SWAPCHAIN_EXTENSION_NAME}},
	.swapchain_cinfo =
		(VkSwapchainCreateInfoKHR){.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                                    .imageArrayLayers = 1,
                                    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                    .imageFormat =
                                                   VK_FORMAT_B8G8R8A8_UNORM, .presentMode =
                                                   VK_PRESENT_MODE_MAILBOX_KHR, .imageColorSpace =
                                                   VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
                                    .compositeAlpha =
                                                   VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                                    .clipped = VK_TRUE},
	.swapchain_img_view_cinfo = (VkImageViewCreateInfo){.sType =
                                                                    VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                    .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                    .format =
                                                                    VK_FORMAT_B8G8R8A8_UNORM,
                                    .components =
                                                                    {
									    VK_COMPONENT_SWIZZLE_IDENTITY,
									    VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
									    VK_COMPONENT_SWIZZLE_IDENTITY},
                                    .subresourceRange = (VkImageSubresourceRange){.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                                                                          .baseMipLevel =
                                                                                                                  0,
                                                                                                          .levelCount =
                                                                                                                  1,
                                                                                                          .baseArrayLayer =
                                                                                                                  0,
                                                                                                          .layerCount =
                                                                                                                  1}},
	.shader_module_vertex_cinfo =
		(VkShaderModuleCreateInfo){
				    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO},
	.shader_module_fragment_cinfo =
		(VkShaderModuleCreateInfo){
				    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO},
	.shader_stages_cinfo =
		{
				    (VkPipelineShaderStageCreateInfo){.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                                          .stage =
                                                                  VK_SHADER_STAGE_VERTEX_BIT,
                                                          .pName = "main"},
				    (VkPipelineShaderStageCreateInfo){
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
				.pName = "main"}},
	.pipeline_dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT,
                                    VK_DYNAMIC_STATE_SCISSOR},
	.pipeline_dynamic_states_cinfo =
		(VkPipelineDynamicStateCreateInfo){
				    .sType =
				VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
				    .dynamicStateCount = 2,
				    .pDynamicStates    = rend_info.pipeline_dynamic_states},
	.vertex_input_state_cinfo =
		(VkPipelineVertexInputStateCreateInfo){
				    .sType =
				VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
				    .vertexBindingDescriptionCount   = 1,
				    .vertexAttributeDescriptionCount = 2,
				    .pVertexBindingDescriptions =
				&rend_info.vertex_input_binding_desc,
				    .pVertexAttributeDescriptions =
				rend_info.vertex_input_attrib_descs},
	.pipeline_assembly_input_state_cinfo =
		(VkPipelineInputAssemblyStateCreateInfo){
				    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
				    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST},
	.pipeline_viewport_state_cinfo =
		(VkPipelineViewportStateCreateInfo){.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                                    .viewportCount = 1,
                                    .scissorCount  = 1},
	.pipeline_rasterization_state_cinfo =
		(VkPipelineRasterizationStateCreateInfo){.sType =
                                                                 VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                                    .polygonMode = VK_POLYGON_MODE_FILL,
                                    .lineWidth = 1.0f,
                                    .cullMode = VK_CULL_MODE_BACK_BIT,
                                    .frontFace =
                                                                 VK_FRONT_FACE_CLOCKWISE},
	.pipeline_msaa_state_cinfo =
		(VkPipelineMultisampleStateCreateInfo){
				    .sType =
				VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
				    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
				    .minSampleShading     = 1.0f},
	.pipeline_color_blend_attachment_state =
		(VkPipelineColorBlendAttachmentState){.blendEnable = VK_TRUE,
                                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
                                    .srcColorBlendFactor =
                                                              VK_BLEND_FACTOR_SRC_ALPHA,
                                    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                                    .srcAlphaBlendFactor =
                                                              VK_BLEND_FACTOR_ONE},
	.pipeline_color_blend_cinfo =
		(VkPipelineColorBlendStateCreateInfo){.sType =
                                                              VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                                    .logicOp =
                                                              VK_LOGIC_OP_COPY, .attachmentCount = 1,
                                    .pAttachments =
                                                              &rend_info
                                                                       .pipeline_color_blend_attachment_state},
	.pipeline_layout_cinfo =
		(VkPipelineLayoutCreateInfo){
				    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO},
	.pass_color_attachment =
		(VkAttachmentDescription){.format  = VK_FORMAT_B8G8R8A8_UNORM,
                                    .samples = VK_SAMPLE_COUNT_1_BIT,
                                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                    .stencilLoadOp =
                                                  VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                    .stencilStoreOp =
                                                  VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR},
	.pass_color_attachment_ref = (VkAttachmentReference){.layout =
                                                                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
	.subpass_desc =
		(VkSubpassDescription){.colorAttachmentCount = 1,
                                    .pColorAttachments = &rend_info.pass_color_attachment_ref},
	.pass_cinfo = (VkRenderPassCreateInfo){.sType =
                                                       VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                    .attachmentCount = 1,
                                    .pAttachments =
                                                       &rend_info
                                                                .pass_color_attachment,
                                    .subpassCount = 1,
                                    .pSubpasses =
                                                       &rend_info.subpass_desc,
                                    .dependencyCount = 1,
                                    .pDependencies =
                                                       &rend_info.subpass_dep},
	.graphics_pipeline_cinfo =
		(VkGraphicsPipelineCreateInfo){
				    .sType =
				VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, .stageCount = 2,
				    .pStages    = rend_info.shader_stages_cinfo,
				    .pVertexInputState =
				&rend_info.vertex_input_state_cinfo,
				    .pInputAssemblyState =
				&rend_info.pipeline_assembly_input_state_cinfo,
				    .pViewportState =
				&rend_info.pipeline_viewport_state_cinfo,
				    .pRasterizationState = &rend_info.pipeline_rasterization_state_cinfo,
				    .pMultisampleState =
				&rend_info.pipeline_msaa_state_cinfo,
				    .pColorBlendState =
				&rend_info.pipeline_color_blend_cinfo,
				    .pDynamicState =
				&rend_info.pipeline_dynamic_states_cinfo,
				    .basePipelineIndex = -1},
	.fbo_cinfo = (VkFramebufferCreateInfo){.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                                    .attachmentCount = 1,
                                    .layers          = 1},
#if KQ_DEBUG
	.debug_messenger_cinfo =
		(VkDebugUtilsMessengerCreateInfoEXT){
				    .sType =
				VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
				    .messageSeverity =
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
				| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
				| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				    .messageType =
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT},
#endif  /* KQ_DEBUG */
	.cmd_pool_cinfo =
		(VkCommandPoolCreateInfo){.sType =
                                                  VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                                    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT},
	.cmd_buf_allocate_info =
		(VkCommandBufferAllocateInfo){.sType =
                                                      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                    .commandBufferCount =
                                                      KQ_FRAMES_IN_FLIGHT},
	.cmd_buf_begin_info =
		(VkCommandBufferBeginInfo){
				    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO},
	.clear_color = (VkClearValue){{{0.0f, 0.0f, 0.0f, 1.0f}}},
	.pass_begin_info =
		(VkRenderPassBeginInfo){.sType =
                                                VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                                    .clearValueCount = 1,
                                    .pClearValues = &rend_info.clear_color},
	.semaphore_cinfo =
		(VkSemaphoreCreateInfo){
				    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO},
	.fence_cinfo =
		(VkFenceCreateInfo){.sType =
                                            VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT},
	.submit_info = (VkSubmitInfo){.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                                    .waitSemaphoreCount   = 1,
                                    .commandBufferCount   = 1,
                                    .signalSemaphoreCount = 1,
                                    .pWaitDstStageMask =
                                              &rend_info.submit_dst_stage_mask},
	.submit_dst_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	.subpass_dep =
		(VkSubpassDependency){
				    .srcSubpass = VK_SUBPASS_EXTERNAL,
				    .srcStageMask =
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, .dstStageMask =
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT},
	.present_info =
		(VkPresentInfoKHR){.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                                    .waitSemaphoreCount = 1,
                                    .swapchainCount     = 1},
	.vertex_input_binding_desc =
		(VkVertexInputBindingDescription){.stride = sizeof(kq_vertex)},
	.vertex_input_attrib_descs =
		{
				    (VkVertexInputAttributeDescription){.format =
                                                                    VK_FORMAT_R32G32_SFLOAT,
                                                            .offset = offsetof(
								    kq_vertex,
								    position)},
				    (VkVertexInputAttributeDescription){
				.location = 1,
				.format   = VK_FORMAT_R32G32B32_SFLOAT,
				.offset   = offsetof(kq_vertex, color)}},
	.vertex_buf_cinfo =
		(VkBufferCreateInfo){.sType =
                                             VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                    .size = sizeof(kq_vertex) * 3,
                                    .usage =
                                             VK_BUFFER_USAGE_VERTEX_BUFFER_BIT}
};
