#pragma once

#include "VulkanAPIHeader.h"

#include "RenderAPIs/RenderAPI/Interface/RenderTypes.h"

namespace cube
{
    namespace rapi
    {
        // TODO: Format에 맞는 정보들 저장
        static Array<VkFormat, (Uint32)TextureFormat::TextureFormatCount> texFmtToVkFmt;

        void InitTypeConversion();

        static VkFormat TextureFormatToVkFormat(TextureFormat format)
        {
            return texFmtToVkFmt[(Uint32)format];
        }

        static VkSampleCountFlagBits ConvertToVkSampleCountFlagBits(Uint8 sampleCount)
        {
            return (VkSampleCountFlagBits)(sampleCount);
        }
    }
} // namespace cube
