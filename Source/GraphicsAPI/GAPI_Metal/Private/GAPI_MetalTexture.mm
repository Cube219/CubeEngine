#include "GAPI_MetalTexture.h"

#include "Checker.h"
#include "GAPI_Resource.h"
#include "MacOS/MacOSString.h"
#include "MetalDevice.h"
#include "MetalTypes.h"
#include "Platform.h"

namespace cube
{
    namespace gapi
    {
        MetalTexture::MetalTexture(const TextureCreateInfo& createInfo, MetalDevice& device)
            : Texture(createInfo)
            , mDevice(device)
            , mMappedPtr(nullptr)
        { @autoreleasepool {
            const TextureInfo& info = createInfo.textureInfo;

            MTLTextureType type;
            // TODO: Support other texture types
            switch (info.type)
            {
            // case TextureType::Texture1D:
            //     type = MTLTextureType1D;
            //     break;
            // case TextureType::Texture1DArray:
            //     type = MTLTextureType1DArray;
            //     break;
            case TextureType::Texture2D:
                type = MTLTextureType2D;
                break;
             case TextureType::Texture2DArray:
                 type = MTLTextureType2DArray;
                 break;
             case TextureType::TextureCube:
                 type = MTLTextureTypeCube;
                 break;
            // case TextureType::TextureCubeArray:
            //     type = MTLTextureTypeCubeArray;
            //     break;
            // case TextureType::Texture3D:
            //     type = MTLTextureType3D;
            //     break;
            default:
                NOT_IMPLEMENTED();
                type = MTLTextureType2D;
            }
            mTextureType = type;

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
            mTextureUsage = usage;

            MTLResourceOptions resourceOptions = MTLResourceStorageModeShared;
            switch (mUsage)
            {
            case ResourceUsage::GPUOnly:
            case ResourceUsage::CPUtoGPU:
            case ResourceUsage::GPUtoCPU:
                resourceOptions = MTLResourceStorageModeShared;
                break;
            }

            MetalElementFormatInfo formatInfo = GetMetalElementFormatInfo(info.format);
            mPixelFormat = formatInfo.pixelFormat;

            MTLTextureDescriptor* desc = [[MTLTextureDescriptor alloc] init];
            desc.textureType = type;
            desc.pixelFormat = formatInfo.pixelFormat;
            desc.width = info.width;
            desc.height = info.height;
            desc.depth = info.depth;
            desc.mipmapLevelCount = info.mipLevels;
            desc.arrayLength = info.arraySize;
            desc.resourceOptions = resourceOptions;
            desc.usage = usage;
            desc.allowGPUOptimizedContents = (mUsage != ResourceUsage::GPUtoCPU);

            mTexture = [device.GetMTLDevice() newTextureWithDescriptor:desc];
            [desc release];
            CHECK(mTexture);
            mTexture.label = String_Convert<NSString*>(createInfo.debugName);

            // Calculate subresource layouts.
            const Uint32 numSlices = GetNumSlices();
            const Uint32 numSubresources = numSlices * mInfo.mipLevels;
            mSubresourceLayouts.resize(numSubresources);
            Uint32 subresourceIndex = 0;
            Uint64 offset = 0;
            for (Uint32 sliceIndex = 0; sliceIndex < numSlices; ++sliceIndex)
            {
                Uint32 width = mInfo.width;
                Uint32 height = mInfo.height;
                for (Uint32 mipLevel = 0; mipLevel < mInfo.mipLevels;  ++mipLevel)
                {
                    SubresourceLayout& layout = mSubresourceLayouts[subresourceIndex];
                    layout.rowPitch = width * formatInfo.bytes;
                    layout.offset = offset;

                    offset += width * height * formatInfo.bytes;

                    width >>= 1;
                    height >>= 1;
                    subresourceIndex++;
                }
            }
            mTotalSize = offset;
        }}

        MetalTexture::~MetalTexture()
        {
            [mTexture release];
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
                // TODO: Use optimizeContentsForGPUAccess in MTLBlitCommandEncoder.
            case ResourceUsage::CPUtoGPU:
            case ResourceUsage::GPUtoCPU:
            {
                Uint32 subresourceIndex = 0;
                const Uint32 numSlices = GetNumSlices();
                for (Uint32 sliceIndex = 0; sliceIndex < numSlices; ++sliceIndex)
                {
                    Uint32 width = mInfo.width;
                    Uint32 height = mInfo.height;
                    for (Uint32 mipLevel = 0; mipLevel < mInfo.mipLevels; ++mipLevel)
                    {
                        void* ptr = (Byte*)mMappedPtr + mSubresourceLayouts[subresourceIndex].offset;
                        [mTexture
                            replaceRegion:MTLRegionMake2D(0, 0, width, height)
                            mipmapLevel:mipLevel
                            slice:sliceIndex
                            withBytes:ptr
                            bytesPerRow:mSubresourceLayouts[subresourceIndex].rowPitch
                            bytesPerImage:0
                        ];

                        width >>= 1;
                        height >>= 1;
                        subresourceIndex++;
                    }
                }
                free(mMappedPtr);
                mMappedPtr = nullptr;
                break;
            }
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
            CHECK_FORMAT(metalTexture->GetTextureUsage() & MTLTextureUsageShaderRead, "Cannot create MetalTextureSRV. Texture was not created with MTLTextureUsageShaderRead.");

            Uint32 mipLevels = createInfo.mipLevels != SubresourceRange::AllRange ? createInfo.mipLevels : metalTexture->GetMipLevels() - createInfo.firstMipLevel;
            Uint32 sliceSize = createInfo.sliceSize != SubresourceRange::AllRange ? createInfo.sliceSize : metalTexture->GetArraySize() - createInfo.firstSliceIndex;
            if (texture->IsCubemap())
            {
                sliceSize *= 6;
            }
            id<MTLTexture> mtlTexture = metalTexture->GetMTLTexture();
            mSRV = [mtlTexture
                newTextureViewWithPixelFormat:metalTexture->GetPixelFormat()
                textureType:metalTexture->GetTextureType()
                levels:NSMakeRange(createInfo.firstMipLevel, mipLevels)
                slices:NSMakeRange(createInfo.firstSliceIndex, sliceSize)
            ];

            mBindlessId = mSRV.gpuResourceID._impl;
        }

        MetalTextureSRV::~MetalTextureSRV()
        {
            [mSRV release];
        }

        MetalTextureUAV::MetalTextureUAV(const TextureUAVCreateInfo& createInfo, SharedPtr<Texture> texture, MetalDevice& device)
            : TextureUAV(createInfo, texture)
            , mDevice(device)
        {
            SharedPtr<MetalTexture> metalTexture = std::dynamic_pointer_cast<MetalTexture>(texture);
            CHECK(metalTexture);

            CHECK_FORMAT(metalTexture->GetTextureUsage() & (MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite), "Cannot create MetalTextureUAV. Texture was not created with MTLTextureUsageShaderRead and MTLTextureUsageShaderWrite.");

            Uint32 sliceSize = createInfo.sliceSize != SubresourceRange::AllRange ? createInfo.sliceSize : metalTexture->GetArraySize() - createInfo.firstSliceIndex;
            if (texture->IsCubemap())
            {
                sliceSize *= 6;
            }
            id<MTLTexture> mtlTexture = metalTexture->GetMTLTexture();
            mUAV = [mtlTexture
                newTextureViewWithPixelFormat:metalTexture->GetPixelFormat()
                textureType:metalTexture->GetTextureType()
                levels:NSMakeRange(createInfo.mipLevel, 1)
                slices:NSMakeRange(createInfo.firstSliceIndex, sliceSize)
            ];

            mBindlessId = mUAV.gpuResourceID._impl;
        }

        MetalTextureUAV::~MetalTextureUAV()
        {
            [mUAV release];
        }

        MetalTextureRTV::MetalTextureRTV(const TextureRTVCreateInfo& createInfo, SharedPtr<Texture> texture, MetalDevice& device)
            : TextureRTV(createInfo, texture)
            , mDevice(device)
        {
            SharedPtr<MetalTexture> metalTexture = std::dynamic_pointer_cast<MetalTexture>(texture);
            CHECK(metalTexture);
            CHECK_FORMAT(metalTexture->GetTextureUsage() & MTLTextureUsageRenderTarget, "Cannot create MetalTextureRTV. Texture was not created with MTLTextureUsageRenderTarget.");

            Uint32 sliceSize = createInfo.sliceSize != SubresourceRange::AllRange ? createInfo.sliceSize : metalTexture->GetArraySize() - createInfo.firstSliceIndex;
            if (texture->IsCubemap())
            {
                sliceSize *= 6;
            }
            mRTV = [metalTexture->GetMTLTexture()
                newTextureViewWithPixelFormat:metalTexture->GetPixelFormat()
                textureType:metalTexture->GetTextureType()
                                       levels:NSMakeRange(createInfo.mipLevel, 1)
                                       slices:NSMakeRange(createInfo.firstSliceIndex, sliceSize)
            ];
        }

        MetalTextureRTV::~MetalTextureRTV()
        {
            if (!mFromExisted)
            {
                [mRTV release];
            }
        }

        MetalTextureRTV::MetalTextureRTV(id<MTLTexture> mtlRTV, MetalDevice& device)
            : mDevice(device)
        {
            mFromExisted = true;

            mRTV = mtlRTV;
        }

        MetalTextureDSV::MetalTextureDSV(const TextureDSVCreateInfo& createInfo, SharedPtr<Texture> texture, MetalDevice& device)
            : TextureDSV(createInfo, texture)
            , mDevice(device)
        {
            SharedPtr<MetalTexture> metalTexture = std::dynamic_pointer_cast<MetalTexture>(texture);
            CHECK(metalTexture);
            CHECK_FORMAT(metalTexture->GetTextureUsage() & MTLTextureUsageRenderTarget, "Cannot create MetalTextureDSV. Texture was not created with MTLTextureUsageRenderTarget.");

            Uint32 sliceSize = createInfo.sliceSize != SubresourceRange::AllRange ? createInfo.sliceSize : metalTexture->GetArraySize() - createInfo.firstSliceIndex;
            if (texture->IsCubemap())
            {
                sliceSize *= 6;
            }
            mDSV = [metalTexture->GetMTLTexture()
                newTextureViewWithPixelFormat:metalTexture->GetPixelFormat()
                                  textureType:metalTexture->GetTextureType()
                                       levels:NSMakeRange(createInfo.mipLevel, 1)
                                       slices:NSMakeRange(createInfo.firstSliceIndex, sliceSize)
            ];
        }

        MetalTextureDSV::~MetalTextureDSV()
        {
            [mDSV release];
        }
    } // namespace gapi
} // namespace cube
