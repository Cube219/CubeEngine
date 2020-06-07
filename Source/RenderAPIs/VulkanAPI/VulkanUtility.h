#pragma once

#include <vulkan/vulkan.h>

#include "Utility/String.h"
#include "Utility/Format.h"
#include "Core/Allocator/FrameAllocator.h"

#ifdef _DEBUG

namespace cube
{
    namespace rapi
    {
        void VkFailed(VkResult res, const char* fileName, int lineNum, StringView msg);

        template <typename ...Args>
        void VkFailedFormatting(VkResult res, const char* fileName, int lineNum, StringView msg, Args&& ...args)
        {
            VkFailed(res, fileName, lineNum, FrameFormat(msg.data(), std::forward<Args>(args)...));
        }

        const Character* GetVkResultString(VkResult res);
    } // namespace rapi
} // namespace cube

#define CHECK_VK(res, msg, ...) if(res != VK_SUCCESS) cube::rapi::VkFailedFormatting(res, __FILE__, __LINE__, CUBE_T(msg), ##__VA_ARGS__);

#else // _DEBUG

#define CHECK_VK(res, msg, ...)

#endif // _DEBUG
