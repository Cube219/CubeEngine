#include "GAPI_MetalBuffer.h"

#include "Checker.h"
#include "MacOS/MacOSString.h"
#include "MetalDevice.h"

namespace cube
{
    namespace gapi
    {
        MetalBuffer::MetalBuffer(const BufferCreateInfo& info, MetalDevice& device)
            : Buffer(info)
            , mDevice(device)
        {
            MTLResourceOptions resourceOptions = MTLResourceStorageModeShared;
            switch (info.usage)
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

            if (info.usage == ResourceUsage::Transient)
            {
                mBuffer = [device.GetTransientHeapManager().GetMTLHeap(MTLSizeAndAlign(mInfo.size, 0)) newBufferWithLength:mInfo.size options:resourceOptions];
            }
            else
            {
                mBuffer = [device.GetMTLDevice() newBufferWithLength:mInfo.size options:resourceOptions];
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
        {
            const Uint64 offset = mFirstElement * metalBuffer->GetStride();

            mBindlessId = metalBuffer->GetMTLBuffer().gpuAddress + offset;
        }

        MetalBufferUAV::MetalBufferUAV(MetalDevice& device, const BufferUAVCreateInfo& createInfo, SharedPtr<MetalBuffer> metalBuffer)
            : BufferUAV(createInfo, metalBuffer)
        {
            const Uint64 offset = mFirstElement * metalBuffer->GetStride();

            mBindlessId = metalBuffer->GetMTLBuffer().gpuAddress + offset;
        }
    } // namespace gapi
} // namespace cube
