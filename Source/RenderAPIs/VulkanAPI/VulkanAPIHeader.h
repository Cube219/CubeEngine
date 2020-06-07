#pragma once
#pragma warning(disable:4251)
#pragma warning(disable:4275)

#include "Utility/Types.h"
#include "VulkanAPIFwd.h"

#include <vulkan/vulkan.h>

#ifdef VK_API_EXPORTS
#define VK_API_EXPORT __declspec(dllexport) 
#else // VK_API_EXPORTS
#define VK_API_EXPORT __declspec(dllimport) 
#endif // VK_API_EXPORTS
