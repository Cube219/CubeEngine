#include "MetalDevice.h"

#include "Logger.h"
#include "MacOS/MacOSString.h"

namespace cube
{
    MetalDevice::MetalDevice()
    {
    }

    MetalDevice::~MetalDevice()
    {
    }

    void MetalDevice::Initialize(id<MTLDevice> device, Uint32 numGPUSync)
    {
        mDevice = device;

        mNumGPUSync = numGPUSync;
        mGPUSyncEvent = [mDevice newSharedEvent];
        mGPUSyncEvent.label = @"GPUSyncEvent";
        mGPUSyncEvent.signaledValue = 0;

        mMainCommandQueue = [device newCommandQueue];
        mMainCommandQueue.label = @"MainCommandQueue";
    }

    void MetalDevice::Shutdown()
    {
        WaitAllGPUSync();

        [mMainCommandQueue release];

        [mDevice release];
    }

    bool MetalDevice::CheckFeatureRequirements()
    {
        // Apple4 / Mac2 (Nonuniform threadgroup size)
        if (![mDevice supportsFamily:MTLGPUFamilyApple4])
        {
            CUBE_LOG(Info, Metal, "Device {0} does not support Apple4 GPU family, which is required.", mDevice.name);
            return false;
        }

        // Argument Buffer Tier2 (For bindless)
        if (mDevice.argumentBuffersSupport < MTLArgumentBuffersTier2)
        {
            const int tierValueOffset = 1 - MTLArgumentBuffersTier1;
            CUBE_LOG(Info, Metal, "Device {0} does not support Argument Buffer Tier 2 (Maximum: Tier{1}), which is required.", mDevice.name, (int)(mDevice.argumentBuffersSupport) + tierValueOffset);
            return false;
        }

        return true;
    }

    void MetalDevice::SetNumGPUSync(Uint32 newNumGPUSync)
    {
        WaitAllGPUSync();

        mNumGPUSync = newNumGPUSync;
    }

    void MetalDevice::BeginGPUFrame(Uint64 gpuFrame)
    {
        if (gpuFrame >= mNumGPUSync)
        {
            [mGPUSyncEvent waitUntilSignaledValue:gpuFrame-1 timeoutMS:100000000000];
        }
    }

    void MetalDevice::EndGPUFrame(Uint64 gpuFrame)
    { @autoreleasepool {
        id<MTLCommandBuffer> signalCommandBuffer = [mMainCommandQueue commandBuffer];
        signalCommandBuffer.label = @"EndGPUFrameSignalCommandBuffer";

        [signalCommandBuffer encodeSignalEvent:mGPUSyncEvent value:gpuFrame];
        [signalCommandBuffer commit];
    }}

    void MetalDevice::WaitAllGPUSync()
    { @autoreleasepool {
        id<MTLCommandBuffer> waitAllGPUSyncCommandBuffer = [mMainCommandQueue commandBuffer];
        waitAllGPUSyncCommandBuffer.label = @"WaitAllGPUSyncCommandBuffer";

        [waitAllGPUSyncCommandBuffer commit];
        [waitAllGPUSyncCommandBuffer waitUntilCompleted];
    }}
} // namespace cube
