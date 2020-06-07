#pragma once

#include "VulkanAPIHeader.h"

#include "RenderAPIs/RenderAPI/Interface/RenderTypes.h"

namespace cube
{
    namespace rapi
    {
        static Array<VkFormat, (Uint32)TextureFormat::TextureFormatCount> texFmtToVkFmt;

        void InitTypeConversion();

        static VkFormat TextureFormatToVkFormat(TextureFormat format)
        {
            return texFmtToVkFmt[(Uint32)format];
        }
    }
} // namespace cube
