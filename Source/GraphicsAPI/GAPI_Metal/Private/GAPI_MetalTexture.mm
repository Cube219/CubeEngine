#include "GAPI_MetalTexture.h"

#include "Checker.h"
#include "GAPI_Resource.h"
#include "MacOS/MacOSString.h"
#include "MetalDevice.h"
#include "MetalTypes.h"
#include "MetalUploadManager.h"
#include "Platform.h"

namespace cube
{
    namespace gapi
    {
        MetalTexture::MetalTexture(const TextureCreateInfo& createInfo, MetalDevice& device, bool skipResourceCreation)
            : Texture(createInfo)
            , mDevice(device)
            , mMappedPtr(nullptr)
            , mFromExisted(false)
        { @autoreleasepool {
            const TextureInfo& info = createInfo.textureInfo;

            CHECK_FORMAT(info.width >= 1, "Texture width must be at least 1. (width: {0})", info.width);
            CHECK_FORMAT(info.mipLevels >= 1, "Texture mipLevels must be at least 1. (mipLevels: {0})", info.mipLevels);

            MTLTextureType type;
            switch (info.type)
            {
            case TextureType::Texture1D:
                CHECK_FORMAT(info.height == 1, "Texture1D must have height = 1. (height: {0})", info.height);
                CHECK_FORMAT(info.depth == 1, "Texture1D must have depth = 1. (depth: {0})", info.depth);
                CHECK_FORMAT(info.arraySize == 1, "Texture1D must have arraySize = 1. (arraySize: {0})", info.arraySize);
                type = MTLTextureType1D;
                break;
            case TextureType::Texture1DArray:
                CHECK_FORMAT(info.height == 1, "Texture1DArray must have height = 1. (height: {0})", info.height);
                CHECK_FORMAT(info.depth == 1, "Texture1DArray must have depth = 1. (depth: {0})", info.depth);
                CHECK_FORMAT(info.arraySize >= 1, "Texture1DArray must have arraySize >= 1. (arraySize: {0})", info.arraySize);
                type = MTLTextureType1DArray;
                break;
            case TextureType::Texture2D:
                CHECK_FORMAT(info.height >= 1, "Texture2D must have height >= 1. (height: {0})", info.height);
                CHECK_FORMAT(info.depth == 1, "Texture2D must have depth = 1. (depth: {0})", info.depth);
                CHECK_FORMAT(info.arraySize == 1, "Texture2D must have arraySize = 1. (arraySize: {0})", info.arraySize);
                type = MTLTextureType2D;
                break;
            case TextureType::Texture2DArray:
                CHECK_FORMAT(info.height >= 1, "Texture2DArray must have height >= 1. (height: {0})", info.height);
                CHECK_FORMAT(info.depth == 1, "Texture2DArray must have depth = 1. (depth: {0})", info.depth);
                CHECK_FORMAT(info.arraySize >= 1, "Texture2DArray must have arraySize >= 1. (arraySize: {0})", info.arraySize);
                type = MTLTextureType2DArray;
                break;
            case TextureType::Texture3D:
                CHECK_FORMAT(info.height >= 1, "Texture3D must have height >= 1. (height: {0})", info.height);
                CHECK_FORMAT(info.depth >= 1, "Texture3D must have depth >= 1. (depth: {0})", info.depth);
                CHECK_FORMAT(info.arraySize == 1, "Texture3D must have arraySize = 1. (arraySize: {0})", info.arraySize);
                type = MTLTextureType3D;
                break;
            case TextureType::TextureCube:
                CHECK_FORMAT(info.height >= 1, "TextureCube must have height >= 1. (height: {0})", info.height);
                CHECK_FORMAT(info.width == info.height, "TextureCube must have width == height. (width: {0}, height: {1})", info.width, info.height);
                CHECK_FORMAT(info.depth == 1, "TextureCube must have depth = 1. (depth: {0})", info.depth);
                CHECK_FORMAT(info.arraySize == 1, "TextureCube must have arraySize = 1. (arraySize: {0})", info.arraySize);
                type = MTLTextureTypeCube;
                break;
            case TextureType::TextureCubeArray:
                CHECK_FORMAT(info.height >= 1, "TextureCubeArray must have height >= 1. (height: {0})", info.height);
                CHECK_FORMAT(info.width == info.height, "TextureCubeArray must have width == height. (width: {0}, height: {1})", info.width, info.height);
                CHECK_FORMAT(info.depth == 1, "TextureCubeArray must have depth = 1. (depth: {0})", info.depth);
                CHECK_FORMAT(info.arraySize >= 1, "TextureCubeArray must have arraySize >= 1. (arraySize: {0})", info.arraySize);
                type = MTLTextureTypeCubeArray;
                break;
            default:
                NOT_IMPLEMENTED();
                type = MTLTextureType2D;
            }
            mMTLTextureType = type;

            MTLTextureUsage usage = MTLTextureUsageShaderRead;
            // Metal treat depth and stencil usage as render target.
            if (info.flags.IsSet(TextureFlag::RenderTarget) || info.flags.IsSet(TextureFlag::DepthStencil))
            {
                usage |= MTLTextureUsageRenderTarget;
            }
            if (info.flags.IsSet(TextureFlag::UAV))
            {
                usage |= MTLTextureUsageShaderWrite;
            }
            mMTLTextureUsage = usage;

            MetalElementFormatInfo formatInfo = GetMetalElementFormatInfo(info.format);
            mMTLPixelFormat = formatInfo.pixelFormat;

            if (skipResourceCreation)
            {
                return;
            }

            MTLResourceOptions resourceOptions = MTLResourceStorageModeShared;
            switch (mUsage)
            {
            case ResourceUsage::GPUOnly:
            case ResourceUsage::CPUtoGPU:
            case ResourceUsage::GPUtoCPU:
                resourceOptions = MTLResourceStorageModeShared;
                break;
            case ResourceUsage::Transient:
                resourceOptions = MTLResourceStorageModePrivate;
                break;
            default:
                NOT_IMPLEMENTED();
                break;
            }

            MTLTextureDescriptor* desc = [[MTLTextureDescriptor alloc] init];
            desc.textureType = mMTLTextureType;
            desc.pixelFormat = mMTLPixelFormat;
            desc.width = info.width;
            desc.height = info.height;
            desc.depth = info.depth;
            desc.mipmapLevelCount = info.mipLevels;
            desc.arrayLength = info.arraySize; // arraySize is already in cube/array units
            desc.resourceOptions = resourceOptions;
            desc.usage = mMTLTextureUsage;
            desc.allowGPUOptimizedContents = (mUsage != ResourceUsage::GPUtoCPU);

            if (mUsage == ResourceUsage::Transient)
            {
                MTLSizeAndAlign sizeAndAlign = [device.GetMTLDevice() heapTextureSizeAndAlignWithDescriptor:desc];
                mMTLTexture = [device.GetTransientHeapManager().GetMTLHeap(sizeAndAlign) newTextureWithDescriptor:desc];
            }
            else
            {
                mMTLTexture = [device.GetMTLDevice() newTextureWithDescriptor:desc];
            }
            CHECK(mMTLTexture);
            mMTLTexture.label = String_Convert<NSString*>(createInfo.debugName);

            // Calculate subresource layouts.
            const Uint32 numSlices = GetNumSlices();
            const Uint32 numSubresources = numSlices * mInfo.mipLevels;
            mSubresourceLayouts.resize(numSubresources);
            Uint32 subresourceIndex = 0;
            Uint64 offset = 0;
            for (Uint32 sliceIndex = 0; sliceIndex < numSlices; ++sliceIndex)
            {
                Uint32 width  = mInfo.width;
                Uint32 height = mInfo.height;
                Uint32 depth  = mInfo.depth;
                for (Uint32 mipLevel = 0; mipLevel < mInfo.mipLevels; ++mipLevel)
                {
                    SubresourceLayout& layout = mSubresourceLayouts[subresourceIndex];
                    layout.rowPitch = width * formatInfo.bytes;
                    layout.offset   = offset;

                    offset += static_cast<Uint64>(layout.rowPitch) * height * depth;

                    width  = std::max(1u, width  >> 1);
                    height = std::max(1u, height >> 1);
                    depth  = std::max(1u, depth  >> 1);
                    subresourceIndex++;
                }
            }
            mTotalSize = offset;
        }}

        MetalTexture::~MetalTexture()
        {
            mMTLTexture = nil;
        }

        void* MetalTexture::Map()
        {
            CHECK_FORMAT(mMappedPtr == nullptr, "Texture is already mapped.");

            switch (mUsage)
            {
            case ResourceUsage::GPUOnly:
            case ResourceUsage::CPUtoGPU:
            case ResourceUsage::GPUtoCPU:
                mMappedPtr = platform::Platform::Allocate(mTotalSize);
                return mMappedPtr;
            case ResourceUsage::Transient:
                NO_ENTRY_FORMAT("Cannot map transient resource.");
                return nullptr;
            default:
                NOT_IMPLEMENTED();
                return nullptr;
            }
        }

        void MetalTexture::Unmap()
        {
            CHECK_FORMAT(mMappedPtr != nullptr, "Texture is not mapped.");

            switch (mUsage)
            {
            case ResourceUsage::GPUOnly:
            case ResourceUsage::CPUtoGPU:
            case ResourceUsage::GPUtoCPU:
            {
                const Uint32 numSlices = GetNumSlices();
                const bool is1D = (mMTLTextureType == MTLTextureType1D || mMTLTextureType == MTLTextureType1DArray);
                const bool is3D = (mMTLTextureType == MTLTextureType3D);

                Uint32 subresourceIndex = 0;
                for (Uint32 sliceIndex = 0; sliceIndex < numSlices; ++sliceIndex)
                {
                    Uint32 width = mInfo.width;
                    Uint32 height = is1D ? 1 : mInfo.height;
                    Uint32 depth = is3D ? mInfo.depth : 1;
                    for (Uint32 mipLevel = 0; mipLevel < mInfo.mipLevels; ++mipLevel)
                    {
                        const SubresourceLayout& layout = mSubresourceLayouts[subresourceIndex];
                        const void* ptr = (const Byte*)mMappedPtr + layout.offset;

                        MTLRegion region;
                        if (is1D)
                        {
                            region = MTLRegionMake1D(0, width);
                        }
                        else if (is3D)
                        {
                            region = MTLRegionMake3D(0, 0, 0, width, height, depth);
                        }
                        else
                        {
                            region = MTLRegionMake2D(0, 0, width, height);
                        }

                        const NSUInteger bytesPerImage = is3D ? static_cast<NSUInteger>(layout.rowPitch) * height : 0;

                        [mMTLTexture
                            replaceRegion:region
                            mipmapLevel:mipLevel
                            slice:sliceIndex
                            withBytes:ptr
                            bytesPerRow:layout.rowPitch
                            bytesPerImage:bytesPerImage];

                        width = std::max(1u, width >> 1);
                        height = std::max(1u, height >> 1);
                        depth = std::max(1u, depth >> 1);
                        subresourceIndex++;
                    }
                }
                free(mMappedPtr);
                mMappedPtr = nullptr;

                if (mUsage == ResourceUsage::GPUOnly)
                {
                    mDevice.GetUploadManager().SubmitTexture(mMTLTexture, true);
                }
                break;
            }
            case ResourceUsage::Transient:
                NO_ENTRY_FORMAT("Cannot unmap transient resource.");
                break;
            default:
                NOT_IMPLEMENTED();
                break;
            }
        }

        SharedPtr<TextureSRV> MetalTexture::CreateSRV(const TextureSRVCreateInfo& createInfo)
        {
            return std::make_shared<MetalTextureSRV>(createInfo, std::dynamic_pointer_cast<MetalTexture>(shared_from_this()), mDevice);
        }

        SharedPtr<TextureUAV> MetalTexture::CreateUAV(const TextureUAVCreateInfo& createInfo)
        {
            return std::make_shared<MetalTextureUAV>(createInfo, std::dynamic_pointer_cast<MetalTexture>(shared_from_this()), mDevice);
        }

        SharedPtr<TextureRTV> MetalTexture::CreateRTV(const TextureRTVCreateInfo& createInfo)
        {
            return std::make_shared<MetalTextureRTV>(createInfo, shared_from_this(), mDevice);
        }

        SharedPtr<TextureDSV> MetalTexture::CreateDSV(const TextureDSVCreateInfo& createInfo)
        {
            return std::make_shared<MetalTextureDSV>(createInfo, shared_from_this(), mDevice);
        }

        MetalTextureSRV::MetalTextureSRV(const TextureSRVCreateInfo& createInfo, SharedPtr<Texture> texture, MetalDevice& device)
            : TextureSRV(createInfo, texture)
            , mDevice(device)
        {
            SharedPtr<MetalTexture> metalTexture = std::dynamic_pointer_cast<MetalTexture>(texture);
            CHECK(metalTexture);
            CHECK_FORMAT(metalTexture->GetMTLTextureUsage() & MTLTextureUsageShaderRead, "Cannot create MetalTextureSRV. Texture was not created with MTLTextureUsageShaderRead.");

            id<MTLTexture> mtlTexture = metalTexture->GetMTLTexture();
            mSRV = [mtlTexture
                newTextureViewWithPixelFormat:metalTexture->GetPixelFormat()
                textureType:metalTexture->GetMTLTextureType()
                levels:NSMakeRange(mSubresourceRange.firstMipLevel, mSubresourceRange.mipLevels)
                slices:NSMakeRange(mSubresourceRange.firstSliceIndex, mSubresourceRange.sliceSize)
            ];

            mBindlessId = mSRV.gpuResourceID._impl;
            mImTextureID = reinterpret_cast<Uint64>((__bridge void*)mSRV);
        }

        MetalTextureSRV::~MetalTextureSRV()
        {
            mSRV = nil;
        }

        MetalTextureUAV::MetalTextureUAV(const TextureUAVCreateInfo& createInfo, SharedPtr<Texture> texture, MetalDevice& device)
            : TextureUAV(createInfo, texture)
            , mDevice(device)
        {
            SharedPtr<MetalTexture> metalTexture = std::dynamic_pointer_cast<MetalTexture>(texture);
            CHECK(metalTexture);

            CHECK_FORMAT(metalTexture->GetMTLTextureUsage() & (MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite), "Cannot create MetalTextureUAV. Texture was not created with MTLTextureUsageShaderRead and MTLTextureUsageShaderWrite.");

            id<MTLTexture> mtlTexture = metalTexture->GetMTLTexture();
            MTLTextureType mtlTextureType = metalTexture->GetMTLTextureType();
            if (mtlTextureType == MTLTextureTypeCube || mtlTextureType == MTLTextureTypeCubeArray)
            {
                // Use texture 2d array type instead of cubemap type in UAV. Slang does not support RWTextureCube(Array)
                // and it cannot be rendered correctly in cubemap type through texture2d_array in shader.
                mtlTextureType = MTLTextureType2DArray;
            }
            mUAV = [mtlTexture
                newTextureViewWithPixelFormat:metalTexture->GetPixelFormat()
                textureType:mtlTextureType
                levels:NSMakeRange(mSubresourceRange.firstMipLevel, mSubresourceRange.mipLevels)
                slices:NSMakeRange(mSubresourceRange.firstSliceIndex, mSubresourceRange.sliceSize)
            ];

            mBindlessId = mUAV.gpuResourceID._impl;
        }

        MetalTextureUAV::~MetalTextureUAV()
        {
            mUAV = nil;
        }

        MetalTextureRTV::MetalTextureRTV(const TextureRTVCreateInfo& createInfo, SharedPtr<Texture> texture, MetalDevice& device)
            : TextureRTV(createInfo, texture)
            , mDevice(device)
        {
            SharedPtr<MetalTexture> metalTexture = std::dynamic_pointer_cast<MetalTexture>(texture);
            CHECK(metalTexture);
            CHECK_FORMAT(metalTexture->GetMTLTextureUsage() & MTLTextureUsageRenderTarget, "Cannot create MetalTextureRTV. Texture was not created with MTLTextureUsageRenderTarget.");

            mRTV = [metalTexture->GetMTLTexture()
                newTextureViewWithPixelFormat:metalTexture->GetPixelFormat()
                textureType:metalTexture->GetMTLTextureType()
                levels:NSMakeRange(mSubresourceRange.firstMipLevel, mSubresourceRange.mipLevels)
                slices:NSMakeRange(mSubresourceRange.firstSliceIndex, mSubresourceRange.sliceSize)
            ];
        }

        MetalTextureRTV::~MetalTextureRTV()
        {
            mRTV = nil;
        }

        MetalTextureDSV::MetalTextureDSV(const TextureDSVCreateInfo& createInfo, SharedPtr<Texture> texture, MetalDevice& device)
            : TextureDSV(createInfo, texture)
            , mDevice(device)
        {
            SharedPtr<MetalTexture> metalTexture = std::dynamic_pointer_cast<MetalTexture>(texture);
            CHECK(metalTexture);
            CHECK_FORMAT(metalTexture->GetMTLTextureUsage() & MTLTextureUsageRenderTarget, "Cannot create MetalTextureDSV. Texture was not created with MTLTextureUsageRenderTarget.");

            mDSV = [metalTexture->GetMTLTexture()
                newTextureViewWithPixelFormat:metalTexture->GetPixelFormat()
                textureType:metalTexture->GetMTLTextureType()
                levels:NSMakeRange(mSubresourceRange.firstMipLevel, mSubresourceRange.mipLevels)
                slices:NSMakeRange(mSubresourceRange.firstSliceIndex, mSubresourceRange.sliceSize)
            ];
        }

        MetalTextureDSV::~MetalTextureDSV()
        {
            mDSV = nil;
        }
    } // namespace gapi
} // namespace cube
