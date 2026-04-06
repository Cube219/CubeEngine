#include "GAPI_MetalBuffer.h"

#include "Checker.h"
#include "MacOS/MacOSString.h"
#include "MetalDevice.h"

namespace cube
{
    namespace gapi
    {
        MetalBuffer::MetalBuffer(const BufferCreateInfo& info, MetalDevice& device) :
            Buffer(info)
        {
            // TODO: Handle managed resource for Intel-based MAC?
            // TODO: Use MTLResourceStorageModePrivate in GPUOnly
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
                mBuffer = [device.GetTransientHeap().GetMTLHeap() newBufferWithLength:info.size options:resourceOptions];
                if (mBuffer == nil)
                {
                    // Failure may be occurred by OOM. Increase transient heap size and try again.
                    device.GetTransientHeap().IncreaseHeapSize();
                    mBuffer = [device.GetTransientHeap().GetMTLHeap() newBufferWithLength:info.size options:resourceOptions];
                    // Don't try if still have failure.
                }
            }
            else
            {
                mBuffer = [device.GetMTLDevice() newBufferWithLength:info.size options:resourceOptions];
            }
            CHECK(mBuffer);
            mBuffer.label = String_Convert<NSString*>(info.debugName);
        }

        MetalBuffer::~MetalBuffer()
        {
            [mBuffer release];
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
    } // namespace gapi
} // namespace cube
