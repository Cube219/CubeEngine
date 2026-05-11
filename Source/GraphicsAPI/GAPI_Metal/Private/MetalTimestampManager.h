#pragma once

#include "MetalHeader.h"

#include "GAPI_Timestamp.h"

namespace cube
{
    class MetalDevice;

    constexpr Uint32 MetalInvalidSampleIndex = (Uint32)-1;

    class MetalTimestampManager
    {
    public:
        static constexpr int MAX_NUM_SAMPLES = 1024;

    public:
        MetalTimestampManager(MetalDevice& device);
        ~MetalTimestampManager();

        MetalTimestampManager(const MetalTimestampManager& other) = delete;
        MetalTimestampManager& operator=(const MetalTimestampManager& rhs) = delete;

        void Initialize(Uint32 numGPUSync);
        void Shutdown();

        void SetNumGPUSync(Uint32 newNumGPUSync);
        void MoveToNextGPUSync(Uint64 nextGPUFrame);

        bool IsSupported() const { return mIsSupported; }

        gapi::TimestampRangeList GetLastTimestampRangeList() const { return mLastTimestampRangeList; }

        id<MTLCounterSampleBuffer> GetCurrentCounterSampleBuffer() const { return mCounterSampleBuffers[mCurrentGPUSyncIndex]; }
        Uint32 GetCurrentLastSampleIndexAndUse(Uint32 numUseSample);
        void AddTimestampRange(StringView name, Uint32 beginSampleIndex, Uint32 endSampleIndex);

    private:
        void UpdateLastTimestamp(Uint64 gpuFrame);

        MetalDevice& mDevice;

        bool mIsSupported = false;
        Uint64 mTimeFrequency;

        Uint32 mCurrentGPUSyncIndex;

        Vector<id<MTLCounterSampleBuffer>> mCounterSampleBuffers;
        Vector<Uint32> mLastSampleIndices;

        gapi::TimestampRangeList mLastTimestampRangeList;

        struct MetalTimestampRange
        {
            String name;
            Uint32 beginSampleIndex;
            Uint32 endSampleIndex;
        };
        Vector<Vector<MetalTimestampRange>> mTimestampRanges;
    };
} // namespace cube
