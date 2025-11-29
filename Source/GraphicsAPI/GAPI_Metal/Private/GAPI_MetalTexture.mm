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
        MetalTexture::MetalTexture(const TextureCreateInfo& info, MetalDevice& device)
            : Texture(info)
            , mDevice(device)
            , mMappedPtr(nullptr)
        { @autoreleasepool {
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
            // case TextureType::Texture2DArray:
            //     type = MTLTextureType2DArray;
            //     break;
            // case TextureType::TextureCube:
            //     type = MTLTextureTypeCube;
            //     break;
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

            // TODO: Use MTLResourceStorageModePrivate in GPUOnly
            MTLResourceOptions resourceOptions = MTLResourceStorageModeShared;
            switch (info.usage)
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
            desc.allowGPUOptimizedContents = (info.usage != ResourceUsage::GPUtoCPU);

            mRowPitch = info.width * formatInfo.bytes;
            mTexture = [device.GetMTLDevice() newTextureWithDescriptor:desc];
            CHECK(mTexture);
            mTexture.label = String_Convert<NSString*>(info.debugName);

            mTotalSize = mRowPitch * info.height * info.depth * info.arraySize;
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
            case ResourceUsage::CPUtoGPU:
            case ResourceUsage::GPUtoCPU:
                [mTexture
                    replaceRegion:MTLRegionMake2D(0, 0, mWidth, mHeight)
                    mipmapLevel:0
                    withBytes:mMappedPtr
                    bytesPerRow:mRowPitch
                ];
                free(mMappedPtr);
                mMappedPtr = nullptr;
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
            CHECK_FORMAT(metalTexture->GetTextureUsage() & MTLTextureUsageShaderRead, "Cannot create MetalTextureSRV. Texture was not created with MTLTextureUsageShaderRead.");

            Uint32 mipLevels = createInfo.mipLevels != -1 ? createInfo.mipLevels : metalTexture->GetMipLevels() - createInfo.firstMipLevel;
            Uint32 arraySize = createInfo.arraySize != -1 ? createInfo.arraySize : metalTexture->GetArraySize() - createInfo.firstArrayIndex;
            mSRV = [metalTexture->GetMTLTexture()
                newTextureViewWithPixelFormat:metalTexture->GetPixelFormat()
                textureType:metalTexture->GetTextureType()
                levels:NSMakeRange(createInfo.firstMipLevel, mipLevels)
                slices:NSMakeRange(createInfo.firstArrayIndex, arraySize)
            ];

            mArgumentBufferHandle = mDevice.GetArgumentBufferManager().Allocate();
            mBindlessIndex = mArgumentBufferHandle.index;
        }

        MetalTextureSRV::~MetalTextureSRV()
        {
            mDevice.GetArgumentBufferManager().Free(mArgumentBufferHandle);
            [mSRV release];
        }

        MetalTextureUAV::MetalTextureUAV(const TextureUAVCreateInfo& createInfo, SharedPtr<Texture> texture, MetalDevice& device)
            : TextureUAV(createInfo, texture)
            , mDevice(device)
        {
            SharedPtr<MetalTexture> metalTexture = std::dynamic_pointer_cast<MetalTexture>(texture);
            CHECK(metalTexture);

            CHECK_FORMAT(metalTexture->GetTextureUsage() & (MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite), "Cannot create MetalTextureUAV. Texture was not created with MTLTextureUsageShaderRead and MTLTextureUsageShaderWrite.");

            Uint32 arraySize = createInfo.arraySize != -1 ? createInfo.arraySize : metalTexture->GetArraySize() - createInfo.firstArrayIndex;
            mUAV = [metalTexture->GetMTLTexture()
                newTextureViewWithPixelFormat:metalTexture->GetPixelFormat()
                textureType:metalTexture->GetTextureType()
                levels:NSMakeRange(createInfo.mipLevel, 1)
                slices:NSMakeRange(createInfo.firstArrayIndex, arraySize)
            ];

            mArgumentBufferHandle = mDevice.GetArgumentBufferManager().Allocate();
            mBindlessIndex = mArgumentBufferHandle.index;
        }

        MetalTextureUAV::~MetalTextureUAV()
        {
            mDevice.GetArgumentBufferManager().Free(mArgumentBufferHandle);
            [mUAV release];
        }

        MetalTextureRTV::MetalTextureRTV(const TextureRTVCreateInfo& createInfo, SharedPtr<Texture> texture, MetalDevice& device)
            : TextureRTV(createInfo, texture)
            , mDevice(device)
        {
            SharedPtr<MetalTexture> metalTexture = std::dynamic_pointer_cast<MetalTexture>(texture);
            CHECK(metalTexture);
            CHECK_FORMAT(metalTexture->GetTextureUsage() & MTLTextureUsageRenderTarget, "Cannot create MetalTextureRTV. Texture was not created with MTLTextureUsageRenderTarget.");

            Uint32 arraySize = createInfo.arraySize != -1 ? createInfo.arraySize : metalTexture->GetArraySize() - createInfo.firstArrayIndex;
            mRTV = [metalTexture->GetMTLTexture()
                newTextureViewWithPixelFormat:metalTexture->GetPixelFormat()
                textureType:metalTexture->GetTextureType()
                                       levels:NSMakeRange(createInfo.mipLevel, 1)
                                       slices:NSMakeRange(createInfo.firstArrayIndex, arraySize)
            ];

            mArgumentBufferHandle = mDevice.GetArgumentBufferManager().Allocate();
        }

        MetalTextureRTV::~MetalTextureRTV()
        {
            mDevice.GetArgumentBufferManager().Free(mArgumentBufferHandle);
            [mRTV release];
        }

        MetalTextureDSV::MetalTextureDSV(const TextureDSVCreateInfo& createInfo, SharedPtr<Texture> texture, MetalDevice& device)
            : TextureDSV(createInfo, texture)
            , mDevice(device)
        {
            SharedPtr<MetalTexture> metalTexture = std::dynamic_pointer_cast<MetalTexture>(texture);
            CHECK(metalTexture);
            CHECK_FORMAT(metalTexture->GetTextureUsage() & MTLTextureUsageRenderTarget, "Cannot create MetalTextureDSV. Texture was not created with MTLTextureUsageRenderTarget.");

            Uint32 arraySize = createInfo.arraySize != -1 ? createInfo.arraySize : metalTexture->GetArraySize() - createInfo.firstArrayIndex;
            mDSV = [metalTexture->GetMTLTexture()
                newTextureViewWithPixelFormat:metalTexture->GetPixelFormat()
                                  textureType:metalTexture->GetTextureType()
                                       levels:NSMakeRange(createInfo.mipLevel, 1)
                                       slices:NSMakeRange(createInfo.firstArrayIndex, arraySize)
            ];

            mArgumentBufferHandle = mDevice.GetArgumentBufferManager().Allocate();
        }

        MetalTextureDSV::~MetalTextureDSV()
        {
            mDevice.GetArgumentBufferManager().Free(mArgumentBufferHandle);
            [mDSV release];
        }
    } // namespace gapi
} // namespace cube
