#include "MetalUploadManager.h"

#include "MetalDevice.h"

namespace cube
{
    MetalUploadManager::MetalUploadManager(MetalDevice& device) :
        mDevice(device)
    {
        mLastFenceValue = 0;
    }

    void MetalUploadManager::Initialize()
    {
        mFenceEvent = [mDevice.GetMTLDevice() newSharedEvent];
        mFenceEvent.label = @"UploadManagerFenceEvent";
        mFenceEvent.signaledValue = 0;
    }

    void MetalUploadManager::Shutdown()
    {
        Wait(mLastFenceValue);

        mFenceEvent = nil;
    }

    MetalFenceValue MetalUploadManager::SubmitTexture(id<MTLTexture> texture, bool waitForCompletion)
    { @autoreleasepool {
        id<MTLCommandBuffer> commandBuffer = [mDevice.GetMainCommandQueue() commandBuffer];
        commandBuffer.label = @"UploadManager_OptimizeTexture";

        id<MTLBlitCommandEncoder> blitEncoder = [commandBuffer blitCommandEncoder];
        [blitEncoder optimizeContentsForGPUAccess:texture];
        [blitEncoder endEncoding];

        mLastFenceValue++;
        [commandBuffer encodeSignalEvent:mFenceEvent value:mLastFenceValue];
        [commandBuffer commit];

        if (waitForCompletion)
        {
            Wait(mLastFenceValue);
        }

        return mLastFenceValue;
    }}

    void MetalUploadManager::Wait(MetalFenceValue fenceValue)
    {
        if (fenceValue > 0 && mFenceEvent.signaledValue < fenceValue)
        {
            [mFenceEvent waitUntilSignaledValue:fenceValue timeoutMS:100000000000];
        }
    }

    bool MetalUploadManager::IsUploadFinished(MetalFenceValue fenceValue)
    {
        return fenceValue <= mFenceEvent.signaledValue;
    }
} // namespace cube
