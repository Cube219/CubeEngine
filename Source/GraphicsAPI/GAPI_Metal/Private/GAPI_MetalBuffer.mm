#include "GAPI_MetalBuffer.h"

#include "Allocator/AllocatorUtility.h"
#include "Checker.h"
#include "MacOS/MacOSString.h"
#include "MetalDevice.h"
#include "MetalTypes.h"

namespace cube
{
    namespace gapi
    {
        MetalBuffer::MetalBuffer(const BufferCreateInfo& info, MetalDevice& device)
            : Buffer(info)
            , mDevice(device)
        {
            mMTLResourceOptions = MTLResourceStorageModeShared;
            switch (info.usage)
            {
            case ResourceUsage::GPUOnly:
            case ResourceUsage::CPUtoGPU:
            case ResourceUsage::GPUtoCPU:
                mMTLResourceOptions = MTLResourceStorageModeShared;
                break;
            case ResourceUsage::Transient:
                mMTLResourceOptions = MTLResourceStorageModePrivate;
                break;
            default:
                NOT_IMPLEMENTED();
                break;
            }

            if (info.usage == ResourceUsage::Transient)
            {
                mBuffer = [device.GetTransientHeapManager().GetMTLHeap(MTLSizeAndAlign(mInfo.size, 0)) newBufferWithLength:mInfo.size options:mMTLResourceOptions];
            }
            else
            {
                mBuffer = [device.GetMTLDevice() newBufferWithLength:mInfo.size options:mMTLResourceOptions];
            }
            CHECK(mBuffer);
            mBuffer.label = String_Convert<NSString*>(info.debugName);
        }

        MetalBuffer::~MetalBuffer()
        {
            mBuffer = nil;
        }

        SharedPtr<BufferSRV> MetalBuffer::CreateSRV(const BufferSRVCreateInfo& createInfo)
        {
            return std::make_shared<MetalBufferSRV>(mDevice, createInfo, shared_from_this());
        }

        SharedPtr<BufferUAV> MetalBuffer::CreateUAV(const BufferUAVCreateInfo& createInfo)
        {
            return std::make_shared<MetalBufferUAV>(mDevice, createInfo, shared_from_this());
        }

        void* MetalBuffer::Map()
        {
            switch (mUsage)
            {
            case ResourceUsage::GPUOnly:
            case ResourceUsage::CPUtoGPU:
            case ResourceUsage::GPUtoCPU:
                return mBuffer.contents;
            case ResourceUsage::Transient:
                NO_ENTRY_FORMAT("Cannot map transient resource.");
                return nullptr;
            default:
                NOT_IMPLEMENTED();
                return nullptr;
            }
            return mBuffer.contents;
        }

        void MetalBuffer::Unmap()
        {
            switch (mUsage)
            {
            case ResourceUsage::GPUOnly:
            case ResourceUsage::CPUtoGPU:
            case ResourceUsage::GPUtoCPU:
                // Do nothing
                break;
            case ResourceUsage::Transient:
                NO_ENTRY_FORMAT("Cannot unmap transient resource.");
                break;
            default:
                NOT_IMPLEMENTED();
                break;
            }
        }

        MetalBufferSRV::MetalBufferSRV(MetalDevice& device, const BufferSRVCreateInfo& createInfo, SharedPtr<MetalBuffer> metalBuffer)
            : BufferSRV(createInfo, metalBuffer)
            , mParentMTLBuffer(metalBuffer->GetMTLBuffer())
        {
            // Slang uses texture_buffer in Buffer.
            if (metalBuffer->GetType() == BufferType::Typed)
            { @autoreleasepool {
                const MetalElementFormatInfo formatInfo = GetMetalElementFormatInfo(createInfo.typedFormat);
                const Uint64 alignment = [device.GetMTLDevice() minimumLinearTextureAlignmentForPixelFormat:formatInfo.pixelFormat];
                const Uint64 alignedOffset = Align(GetOffset(), alignment);
                const Uint64 size = GetSize();
                CHECK_FORMAT(alignedOffset + size <= metalBuffer->GetSize(), "Aligned offset exceeded the buffer boundary!");

                MTLTextureDescriptor* desc = [[MTLTextureDescriptor alloc] init];
                desc.textureType = MTLTextureTypeTextureBuffer;
                desc.pixelFormat = formatInfo.pixelFormat;
                desc.width = size / formatInfo.bytes;
                desc.resourceOptions = metalBuffer->GetMTLResourceOptions();
                desc.allowGPUOptimizedContents = (metalBuffer->GetUsage() != ResourceUsage::GPUtoCPU);

                mTypedTextureBuffer = [mParentMTLBuffer
                    newTextureWithDescriptor:desc
                    offset:alignedOffset
                    bytesPerRow:size
                ];

                mBindlessId = mTypedTextureBuffer.gpuResourceID._impl;
            }}
            else
            {
                mTypedTextureBuffer = nil;

                const Uint64 offset = mFirstElement * metalBuffer->GetStride();
                mBindlessId = mParentMTLBuffer.gpuAddress + offset;
            }
        }

        MetalBufferUAV::MetalBufferUAV(MetalDevice& device, const BufferUAVCreateInfo& createInfo, SharedPtr<MetalBuffer> metalBuffer)
            : BufferUAV(createInfo, metalBuffer)
            , mParentMTLBuffer(metalBuffer->GetMTLBuffer())
        {
            // Slang uses texture_buffer in RWBuffer.
            if (metalBuffer->GetType() == BufferType::Typed)
            { @autoreleasepool {
                const MetalElementFormatInfo formatInfo = GetMetalElementFormatInfo(createInfo.typedFormat);
                const Uint64 alignment = [device.GetMTLDevice() minimumLinearTextureAlignmentForPixelFormat:formatInfo.pixelFormat];
                const Uint64 alignedOffset = Align(GetOffset(), alignment);
                const Uint64 size = GetSize();
                CHECK_FORMAT(alignedOffset + size <= metalBuffer->GetSize(), "Aligned offset exceeded the buffer boundary!");

                MTLTextureDescriptor* desc = [[MTLTextureDescriptor alloc] init];
                desc.textureType = MTLTextureTypeTextureBuffer;
                desc.pixelFormat = formatInfo.pixelFormat;
                desc.width = size / formatInfo.bytes;
                desc.resourceOptions = metalBuffer->GetMTLResourceOptions();
                desc.allowGPUOptimizedContents = (metalBuffer->GetUsage() != ResourceUsage::GPUtoCPU);

                mTypedTextureBuffer = [mParentMTLBuffer
                    newTextureWithDescriptor:desc
                    offset:alignedOffset
                    bytesPerRow:size
                ];

                mBindlessId = mTypedTextureBuffer.gpuResourceID._impl;
            }}
            else
            {
                mTypedTextureBuffer = nil;

                const Uint64 offset = mFirstElement * metalBuffer->GetStride();
                mBindlessId = mParentMTLBuffer.gpuAddress + offset;
            }
        }
    } // namespace gapi
} // namespace cube
