#pragma once

#include "GAPIHeader.h"

#include "CubeString.h"

namespace cube
{
    namespace gapi
    {
        enum class ResourceType
        {
            Buffer,
            Texture
        };

        enum class ResourceUsage
        {
            GPUOnly,
            CPUtoGPU,
            GPUtoCPU
        };
        inline const Character* ResourceUsageToString(ResourceUsage resourceUsage)
        {
            switch (resourceUsage)
            {
            case ResourceUsage::GPUOnly:
                return CUBE_T("GPUOnly");
            case ResourceUsage::CPUtoGPU:
                return CUBE_T("CPUtoGPU");
            case ResourceUsage::GPUtoCPU:
                return CUBE_T("GPUtoCPU");
            default:
                return CUBE_T("Unknown");
            }
        }

        enum class ResourceStateFlag
        {
            Common = 0,
            Vertex = (1 << 0),
            Index = (1 << 1),
            RenderTarget = (1 << 2),
            SRV_Pixel = (1 << 3),
            SRV_NonPixel = (1 << 4),
            UAV = (1 << 5),
            DepthRead = (1 << 6),
            DepthWrite = (1 << 7),
            IndirectArgs = (1 << 8),
            CopySrc = (1 << 9),
            CopyDst = (1 << 10),
            ResolveSrc = (1 << 11),
            ResolveDst = (1 << 12),
            Present = (1 << 15),
        };
        using ResourceStateFlags = Flags<ResourceStateFlag>;
        FLAGS_OPERATOR(ResourceStateFlag);

        struct SubresourceRange
        {
            Uint32 firstMipLevel;
            Uint32 mipLevels;

            Uint32 firstArrayIndex;
            Uint32 arraySize;
        };
    } // namespace gapi
} // namespace cube
