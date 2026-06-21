#pragma once

#include "GAPIHeader.h"

#include "CubeString.h"

namespace cube
{
    namespace gapi
    {
        class Buffer;
        class Texture;
        struct TextureInfo;

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

        // TODO: Add raytracing related flags.
        enum class ResourceSyncFlag
        {
            None = 0,
            All = (1 << 0),
            Index = (1 << 1),
            Vertex = (1 << 2),
            Pixel = (1 << 3),
            DepthStencil = (1 << 4),
            RenderTarget = (1 << 5),
            Compute = (1 << 6),
            Copy = (1 << 7),
            Resolve = (1 << 8),
            ExecuteIndirect = (1 << 9),
            ClearUAV = (1 << 10),
        };
        using ResourceSyncFlags = Flags<ResourceSyncFlag>;
        FLAGS_OPERATOR(ResourceSyncFlag);

        // TODO: Add raytracing related flags.
        enum class ResourceAccessFlag
        {
            Common = 0,
            NoAccess = (1 << 0),
            ConstantBuffer = (1 << 1),
            VertexBuffer = (1 << 2),
            IndexBuffer = (1 << 3),
            RenderTarget = (1 << 4),
            SRV = (1 << 5),
            UAV = (1 << 6),
            DepthStencilRead = (1 << 7),
            DepthStencilWrite = (1 << 8),
            CopySrc = (1 << 9),
            CopyDst = (1 << 10),
            ResolveSrc = (1 << 11),
            ResolveDst = (1 << 12),
        };
        using ResourceAccessFlags = Flags<ResourceAccessFlag>;
        FLAGS_OPERATOR(ResourceAccessFlag);

        // TODO: Add raytracing related layouts.
        enum class ResourceLayout
        {
            Undefined,
            Common,
            Common_Direct,
            Common_Async,
            Present,
            GenericRead,
            GenericRead_Direct,
            GenericRead_Async,
            RenderTarget,
            SRV,
            SRV_Direct,
            SRV_Async,
            UAV,
            UAV_Direct,
            UAV_Async,
            DepthStencilRead,
            DepthStencilWrite,
            CopySrc,
            CopySrc_Direct,
            CopySrc_Async,
            CopyDst,
            CopyDst_Direct,
            CopyDst_Async,
            ResolveSrc,
            ResolveDst,
        };

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

            bool operator==(const SubresourceRange& rhs) const
            {
                return firstMipLevel == rhs.firstMipLevel
                    && mipLevels == rhs.mipLevels
                    && firstSliceIndex == rhs.firstSliceIndex
                    && sliceSize == rhs.sliceSize;
            }

            Uint64 GetHash() const
            {
                return HashCombine(firstMipLevel, mipLevels, firstSliceIndex, sliceSize);
            }

            bool IsOverlap(const SubresourceRange& rhs) const
            {
                if (firstSliceIndex + sliceSize <= rhs.firstSliceIndex || rhs.firstSliceIndex + rhs.sliceSize <= firstSliceIndex)
                {
                    return false;
                }

                if (firstMipLevel + mipLevels <= rhs.firstMipLevel || rhs.firstMipLevel + rhs.mipLevels <= firstMipLevel)
                {
                    return false;
                }
                return true;
            }
        };

        struct SubresourceRangeInput
        {
            static constexpr Uint32 AllRange = std::numeric_limits<Uint32>::max();

            // [firstMipLevel, firstMipLevel + mipLevels - 1]
            Uint32 firstMipLevel = 0;
            Uint32 mipLevels = SubresourceRangeInput::AllRange;

            // [firstSliceIndex, firstSliceIndex + sliceSize - 1]
            Uint32 firstSliceIndex = 0;
            Uint32 sliceSize = SubresourceRangeInput::AllRange;

            SubresourceRange Clamp(const Texture* texture) const;
            SubresourceRange Clamp(const gapi::TextureInfo& textureInfo) const;

            bool IsOverlap(const SubresourceRangeInput& rhs) const
            {
                if ((sliceSize != SubresourceRangeInput::AllRange && firstSliceIndex + sliceSize <= rhs.firstSliceIndex)
                    || (rhs.sliceSize != SubresourceRangeInput::AllRange && rhs.firstSliceIndex + rhs.sliceSize <= firstSliceIndex))
                {
                    return false;
                }

                if ((mipLevels != SubresourceRangeInput::AllRange && firstMipLevel + mipLevels <= rhs.firstMipLevel)
                    || (rhs.mipLevels != SubresourceRangeInput::AllRange && rhs.firstMipLevel + rhs.mipLevels <= firstMipLevel))
                {
                    return false;
                }
                return true;
            }
        };
    } // namespace gapi
} // namespace cube
