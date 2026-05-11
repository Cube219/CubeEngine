#include "MetalTimestampManager.h"

#include "Checker.h"
#include "MacOS/MacOSString.h"
#include "MetalDevice.h"

namespace cube
{
    MetalTimestampManager::MetalTimestampManager(MetalDevice& device)
        : mDevice(device)
    {
    }

    MetalTimestampManager::~MetalTimestampManager()
    {
    }

    void MetalTimestampManager::Initialize(Uint32 numGPUSync)
    {
        if (!mDevice.IsCounterSamplingSupported())
        {
            return;
        }
        mIsSupported = true;

        // Apple Silicon uses nanoseconds.
        mTimeFrequency = 1'000'000'000;

        SetNumGPUSync(numGPUSync);
    }

    void MetalTimestampManager::Shutdown()
    {
        if (!mIsSupported)
        {
            return;
        }

        SetNumGPUSync(0);
    }

    void MetalTimestampManager::SetNumGPUSync(Uint32 newNumGPUSync)
    { @autoreleasepool {
        if (!mIsSupported)
        {
            return;
        }

        mCounterSampleBuffers.clear();

        id<MTLCounterSet> timestampCounterSet = nil;
        for (id<MTLCounterSet> counterSet in mDevice.GetMTLDevice().counterSets)
        {
            if ([counterSet.name isEqualToString:MTLCommonCounterSetTimestamp])
            {
                timestampCounterSet = counterSet;
                break;
            }
        }
        CHECK_FORMAT(timestampCounterSet, "Cannot find timestamp counter set in this GPU.");

        MTLCounterSampleBufferDescriptor* desc = [[MTLCounterSampleBufferDescriptor alloc] init];
        desc.counterSet = timestampCounterSet;
        desc.sampleCount = MAX_NUM_SAMPLES;
        desc.storageMode = MTLStorageModeShared;

        NSError* error = nil;
        mCounterSampleBuffers.resize(newNumGPUSync);
        for (id<MTLCounterSampleBuffer> __strong& buffer : mCounterSampleBuffers)
        {
            buffer = [mDevice.GetMTLDevice() newCounterSampleBufferWithDescriptor:desc error:&error];
            CHECK_FORMAT(error == nil, "Failed create counter sampler buffer. ({0})", [error localizedDescription]);
            CHECK(buffer);
        }

        mTimestampRanges.clear();
        mTimestampRanges.resize(newNumGPUSync);
        mLastSampleIndices.clear();
        mLastSampleIndices.resize(newNumGPUSync, 0);

        mCurrentGPUSyncIndex = 0;
    }}

    void MetalTimestampManager::MoveToNextGPUSync(Uint64 nextGPUFrame)
    {
        if (!mIsSupported)
        {
            return;
        }

        Uint32 numSyncBuffer = mCounterSampleBuffers.size();

        mCurrentGPUSyncIndex = (mCurrentGPUSyncIndex + 1) % numSyncBuffer;

        if (nextGPUFrame >= numSyncBuffer)
        {
            UpdateLastTimestamp(nextGPUFrame - numSyncBuffer);
        }

        mTimestampRanges[mCurrentGPUSyncIndex].clear();
        mLastSampleIndices[mCurrentGPUSyncIndex] = 0;
    }

    Uint32 MetalTimestampManager::GetCurrentLastSampleIndexAndUse(Uint32 numUseSample)
    {
        if (!mIsSupported)
        {
            return 0;
        }

        Uint32 res = mLastSampleIndices[mCurrentGPUSyncIndex];

        mLastSampleIndices[mCurrentGPUSyncIndex] += numUseSample;
        CHECK(mLastSampleIndices[mCurrentGPUSyncIndex] <= MAX_NUM_SAMPLES);

        return res;
    }

    void MetalTimestampManager::AddTimestampRange(StringView name, Uint32 beginSampleIndex, Uint32 endSampleIndex)
    {
        if (!mIsSupported)
        {
            return;
        }

        mTimestampRanges[mCurrentGPUSyncIndex].push_back({
            .name = { name.begin(), name.end() },
            .beginSampleIndex = beginSampleIndex,
            .endSampleIndex = endSampleIndex
        });
    }

    void MetalTimestampManager::UpdateLastTimestamp(Uint64 gpuFrame)
    {
        mLastTimestampRangeList.frame = gpuFrame;
        mLastTimestampRangeList.frequency = mTimeFrequency;

        // Current index is the last index.
        // This function will be called after moving to next GPU sync index so it is guaranteed
        // that the last GPU sync is finished in MetalDevice::BeginGPUFrame().
        const Uint32 lastGPUSyncIndex = mCurrentGPUSyncIndex;

        const Uint32 numSamples = mLastSampleIndices[lastGPUSyncIndex];
        NSData* nsData = [mCounterSampleBuffers[lastGPUSyncIndex] resolveCounterRange:NSMakeRange(0, numSamples)];
        if (nsData)
        {
            Byte* data = (Byte*)nsData.bytes;

            Uint32 resolvedTimestampCount = std::min((Uint32)(nsData.length / sizeof(MTLCounterResultTimestamp)), (Uint32)MAX_NUM_SAMPLES);

            const Vector<MetalTimestampRange>& lastTimestampRanges = mTimestampRanges[lastGPUSyncIndex];

            mLastTimestampRangeList.timestampRanges.resize(lastTimestampRanges.size());
            for (int i = 0; i < lastTimestampRanges.size(); ++i)
            {
                const MetalTimestampRange& metalTimestampRange = lastTimestampRanges[i];
                mLastTimestampRangeList.timestampRanges[i].name = metalTimestampRange.name;

                if (metalTimestampRange.beginSampleIndex == MetalInvalidSampleIndex
                    && metalTimestampRange.endSampleIndex == MetalInvalidSampleIndex)
                {
                    mLastTimestampRangeList.timestampRanges[i].beginTime = 0;
                    mLastTimestampRangeList.timestampRanges[i].endTime = 0;
                }
                else
                {
                    CHECK(metalTimestampRange.beginSampleIndex < resolvedTimestampCount);
                    CHECK(metalTimestampRange.endSampleIndex < resolvedTimestampCount);

                    MTLCounterResultTimestamp* beginTimestamp = (MTLCounterResultTimestamp*)(data + metalTimestampRange.beginSampleIndex * sizeof(MTLCounterResultTimestamp));
                    MTLCounterResultTimestamp* endTimestamp = (MTLCounterResultTimestamp*)(data + metalTimestampRange.endSampleIndex * sizeof(MTLCounterResultTimestamp));

                    mLastTimestampRangeList.timestampRanges[i].beginTime = beginTimestamp->timestamp;
                    mLastTimestampRangeList.timestampRanges[i].endTime = endTimestamp->timestamp;
                }
            }
        }
        else
        {
            CUBE_LOG(Error, Metal, "Failed to resolve counter samples. Cannot update timestamps.");
            mLastTimestampRangeList.timestampRanges.clear();
        }
    }
} // namespace cube
