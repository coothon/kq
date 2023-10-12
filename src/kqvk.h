#ifndef KQVK_H_
#define KQVK_H_

#include <stdbool.h>

#include <glad/vulkan.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <libcbase/common.h>

#include <kq.h>

#if KQ_DEBUG
extern bool kqvk_instance_add_validation_layers(VkInstanceCreateInfo instance_cinfo[static 1]);
#endif
extern int             kqvk_reload_vulkan(VkInstance ins, VkPhysicalDevice pdev, VkDevice ldev);
extern bool            kqvk_instance_add_extensions(VkInstanceCreateInfo instance_cinfo[static 1]);
extern bool            kqvk_choose_pdev(kq_data kq[static 1]);
extern bool            kqvk_check_pdev_for_extension(VkPhysicalDevice pdev, const char ext[restrict static 1]);
extern bool            kqvk_set_up_pdev_queues(kq_data kq[static 1]);
extern void            kqvk_ready_new_resolution(kq_data kq[static 1], int w, int h);
extern bool            kqvk_swapchain_create(kq_data kq[static 1]);
extern bool            kqvk_init_shaders(kq_data kq[static 1]);
extern bool            kqvk_render_pass_create(kq_data kq[static 1]);
extern bool            kqvk_create_descriptor_set_layout(kq_data kq[static 1]);
extern bool            kqvk_pipeline_create(kq_data kq[static 1]);
extern bool            kqvk_framebuffers_create(kq_data kq[static 1]);
extern bool            kqvk_cmd_pool_create(kq_data kq[static 1]);
extern bool            kqvk_cmd_bufs_create(kq_data kq[static 1]);
extern bool            kqvk_uniforms_init(kq_data kq[static 1]);
extern bool            kqvk_vertex_buffer_create(kq_data kq[static 1]);
extern bool            kqvk_index_buffer_create(kq_data kq[static 1]);
extern bool            kqvk_uniform_buffers_create(kq_data kq[static 1]);
extern bool            kqvk_buffer_create(kq_data               kq[restrict static 1],
                                          VkDeviceSize          size,
                                          VkBufferUsageFlags    usage,
                                          VkMemoryPropertyFlags props,
                                          VkBuffer              buf[restrict static 1],
                                          VkDeviceMemory        buf_mem[restrict static 1]);
extern void            kqvk_buffer_copy(kq_data kq[static 1], VkBuffer src, VkBuffer dst, VkDeviceSize size);
extern u32             kqvk_mem_type_find(kq_data kq[static 1], u32 type_filter, VkMemoryPropertyFlags props);
extern bool            kqvk_sync_primitives_create(kq_data kq[static 1]);
extern bool            kqvk_swapchain_recreate(kq_data kq[static 1]);
extern bool            kqvk_cmd_buf_record(kq_data kq[static 1], VkCommandBuffer cmd_buf);
extern void            kqvk_uniforms_update_time(kq_data kq[static 1]);
extern void            kqvk_uniforms_push(kq_data kq[static 1]);
extern bool            kqvk_create_descriptor_sets(kq_data kq[static 1]);
extern bool            kqvk_image_create(kq_data               kq[restrict static 1],
                                         u32                   tw,
                                         u32                   th,
                                         u32                   array_layers,
                                         VkFormat              fmt,
                                         VkImageTiling         tiling,
                                         VkImageUsageFlags     usage,
                                         VkMemoryPropertyFlags props,
                                         VkImage               img[restrict static 1],
                                         VkDeviceMemory        img_mem[restrict static 1]);
extern VkCommandBuffer kqvk_single_time_command_begin(kq_data kq[static 1]);
extern void            kqvk_single_time_command_end(kq_data kq[restrict static 1], VkCommandBuffer cmd_buf);
extern bool            kqvk_image_layout_transition(kq_data       kq[restrict static 1],
                                                    VkImage       img,
                                                    u32           array_layers,
                                                    VkFormat      fmt,
                                                    VkImageLayout old_layout,
                                                    VkImageLayout new_layout);
extern void            kqvk_buffer_copy_to_image(kq_data kq[restrict static 1], VkBuffer buf, VkImage img, u32 width, u32 height, u32 array_layers);
extern bool            kqvk_tex_create(kq_data kq[restrict static 1]);
extern bool            kqvk_tex_view_create(kq_data kq[static 1]);
extern bool            kqvk_tex_sampler_create(kq_data kq[static 1]);

#if KQ_DEBUG
extern vecstr *kqvk_validation_layers_vec;
#endif
extern vecstr *kqvk_instance_exts_vec;

#endif /* KQVK_H_ */
