#pragma once

#include "MetalHeader.h"

#include "GAPI_Timestamp.h"

namespace cube
{
    class MetalDevice;

    class MetalTimestampManager
    {
    public:
        static constexpr int MAX_NUM_TIMESTAMP = 256;

    public:
        MetalTimestampManager(MetalDevice& device);
        ~MetalTimestampManager();

        MetalTimestampManager(const MetalTimestampManager& other) = delete;
        MetalTimestampManager& operator=(const MetalTimestampManager& rhs) = delete;

        void Initialize(Uint32 numGPUSync);
        void Shutdown();

        void SetNumGPUSync(Uint32 newNumGPUSync);
        void MoveToNextIndex(Uint64 nextGPUFrame);

        gapi::TimestampList GetLastTimestampList() const { return mLastTimestampList; }

        id<MTLCounterSampleBuffer> GetCurrentCounterSampleBuffer() const { return mCounterSampleBuffers[mCurrentIndex]; }
        int AddTimestamp(const String& name);

    private:
        void UpdateLastTimestamp(Uint64 gpuFrame);

        MetalDevice& mDevice;

        Uint32 mCurrentIndex;

        Vector<id<MTLCounterSampleBuffer>> mCounterSampleBuffers;

        gapi::TimestampList mLastTimestampList;

        Vector<Vector<String>> mTimestampNames;
    };
} // namespace cube
