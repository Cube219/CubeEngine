#pragma once

#include "MetalHeader.h"

namespace cube
{
    class MetalDevice;

    using MetalFenceValue = Uint64;

    class MetalUploadManager
    {
    public:
        MetalUploadManager(MetalDevice& device);

        MetalUploadManager(const MetalUploadManager& other) = delete;
        MetalUploadManager& operator=(const MetalUploadManager& rhs) = delete;

        void Initialize();
        void Shutdown();

        MetalFenceValue SubmitTexture(id<MTLTexture> texture, bool waitForCompletion = false);

        void Wait(MetalFenceValue fenceValue);
        bool IsUploadFinished(MetalFenceValue fenceValue);

    private:
        MetalDevice& mDevice;

        id<MTLSharedEvent> mFenceEvent;
        MetalFenceValue mLastFenceValue;
    };
} // namespace cube
