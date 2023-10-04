#ifndef KQ_VULKAN_INCLUDE_H_
#define KQ_VULKAN_INCLUDE_H_

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-identifier"
#if KQ_DEBUG
#include "vulkan_dbg.h"
#else
#include "vulkan_rel.h"
#define gladInstallVulkanDebug(...)
#define gladUninstallVulkanDebug(...)
#define gladSetVulkanPreCallback(...)
#define gladSetVulkanPostCallback(...)
#endif
#pragma clang diagnostic pop

#endif /* KQ_VULKAN_INCLUDE_H_*/
