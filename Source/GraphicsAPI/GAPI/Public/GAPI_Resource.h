#pragma once

#include "GAPIHeader.h"

#include "CubeString.h"

namespace cube
{
    namespace gapi
    {
        class Buffer;
        class Texture;

        enum class ResourceType
        {
            Buffer,
            Texture
        };

        enum class ResourceUsage
        {
            GPUOnly,
            CPUtoGPU,
            GPUtoCPU,
            Transient
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
            case ResourceUsage::Transient:
                return CUBE_T("Transient");
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

        struct SubresourceLayout
        {
            Uint64 offset;
            Uint64 rowPitch;
        };

        struct SubresourceRange
        {
            Uint32 firstMipLevel;
            Uint32 mipLevels;

            Uint32 firstSliceIndex;
            Uint32 sliceSize;

            Uint64 GetHash() const
            {
                return HashCombine(firstMipLevel, mipLevels, firstSliceIndex, sliceSize);
            }
        };

        struct SubresourceRangeInput
        {
            Uint32 firstMipLevel = 0;
            Int32 mipLevels = -1;

            Uint32 firstSliceIndex = 0;
            Int32 sliceSize = -1;

            SubresourceRange Clamp(const Buffer* buffer) const;
            SubresourceRange Clamp(const Texture* texture) const;

            SubresourceRangeInput() = default;
            SubresourceRangeInput(const SubresourceRange& other)
                : firstMipLevel(other.firstMipLevel)
                , mipLevels(other.mipLevels)
                , firstSliceIndex(other.firstSliceIndex)
                , sliceSize(other.sliceSize)
            {
            }
        };
    } // namespace gapi
} // namespace cube
