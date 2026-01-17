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
            }

            mBuffer = [device.GetMTLDevice() newBufferWithLength:info.size options:resourceOptions];
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
            default:
                NOT_IMPLEMENTED();
                break;
            }
        }
    } // namespace gapi
} // namespace cube
