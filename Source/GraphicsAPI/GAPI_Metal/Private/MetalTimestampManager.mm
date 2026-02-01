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
        SetNumGPUSync(numGPUSync);
    }

    void MetalTimestampManager::Shutdown()
    {
        SetNumGPUSync(0);
    }

    void MetalTimestampManager::SetNumGPUSync(Uint32 newNumGPUSync)
    { @autoreleasepool {
        for (id<MTLCounterSampleBuffer> buffer : mCounterSampleBuffers)
        {
            [buffer release];
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
        desc.sampleCount = MAX_NUM_TIMESTAMP;
        desc.storageMode = MTLStorageModeShared;

        NSError* error = nil;
        mCounterSampleBuffers.resize(newNumGPUSync);
        for (id<MTLCounterSampleBuffer>& buffer : mCounterSampleBuffers)
        {
            buffer = [mDevice.GetMTLDevice() newCounterSampleBufferWithDescriptor:desc error:&error];
            if (error != nil)
            {
                CHECK_FORMAT(false, "Failed create counter sampler buffer. ({0})", [error localizedDescription]);
            }
            CHECK(buffer);
        }
        [desc release];

        // Name
        mTimestampNames.clear();
        mTimestampNames.resize(newNumGPUSync);
    }}

    void MetalTimestampManager::MoveToNextIndex(Uint64 nextGPUFrame)
    {
        Uint32 numSyncBuffer = mCounterSampleBuffers.size();

        mCurrentIndex = (mCurrentIndex + 1) % numSyncBuffer;

        if (nextGPUFrame >= numSyncBuffer)
        {
            UpdateLastTimestamp(nextGPUFrame - numSyncBuffer);
        }
    }

    void MetalTimestampManager::UpdateLastTimestamp(Uint64 gpuFrame)
    {
        // Current index will be the last index.
        // This function will be called after moving to next index so it is guaranteed
        // that the last GPU sync related to the index is finished in DX12Device::BeginGPUFrame().
        const Uint32 lastIndex = mCurrentIndex;

        NSData* nsData = [mCounterSampleBuffers[lastIndex] resolveCounterRange:NSMakeRange(0, MAX_NUM_TIMESTAMP)];
        Byte* data = (Byte*)nsData.bytes;

        Uint32 resolvedTimestampCount = nsData.length / sizeof(MTLCounterResultTimestamp);
        resolvedTimestampCount = std::min(resolvedTimestampCount, (Uint32)MAX_NUM_TIMESTAMP);

        const Vector<String>& lastTimestampNames = mTimestampNames[lastIndex];
        CHECK(lastTimestampNames.size() <= resolvedTimestampCount);

        mLastTimestampList.frame = gpuFrame;
        mLastTimestampList.timestamps.resize(lastTimestampNames.size());
        for (int i = 0; i < lastTimestampNames.size(); ++i)
        {
            MTLCounterResultTimestamp* timestamp = (MTLCounterResultTimestamp*)(data + sizeof(MTLCounterResultTimestamp));

            mLastTimestampList.timestamps[i].name = lastTimestampNames[i];
            mLastTimestampList.timestamps[i].time = timestamp->timestamp;
        }
    }

    int MetalTimestampManager::AddTimestamp(const String& name)
    {
        // TODO: Add critical section?
        mTimestampNames[mCurrentIndex].push_back(name);
        const int currentSize = static_cast<int>(mTimestampNames[mCurrentIndex].size());

        CHECK(currentSize <= MAX_NUM_TIMESTAMP);

        return currentSize - 1;
    }
} // namespace cube
