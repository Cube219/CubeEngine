#include "MetalArgumentBufferManager.h"

#include <numeric>

#include "Checker.h"
#include "MetalDevice.h"

namespace cube
{
    MetalArgumentBufferManager::MetalArgumentBufferManager(MetalDevice& device)
        : mDevice(device)
    {
    }

    MetalArgumentBufferManager::~MetalArgumentBufferManager()
    {
    }

    void MetalArgumentBufferManager::Initialize()
    {
        mTotalIndices = 1024;

        mFreedIndices.resize(mTotalIndices);
        std::iota(mFreedIndices.begin(), mFreedIndices.end(), 0);
        std::reverse(mFreedIndices.begin(), mFreedIndices.end());
    }

    void MetalArgumentBufferManager::Shutdown()
    {
        CHECK_FORMAT(mFreedIndices.size() == mTotalIndices, "All argument buffer handle should be freed before shutdown.");
    }

    MetalArgumentBufferHandle MetalArgumentBufferManager::Allocate()
    {
        CHECK(mFreedIndices.size() > 0);

        Uint32 index = mFreedIndices.back();
        mFreedIndices.pop_back();

        return {
            .index = static_cast<int>(index)
        };
    }

    void MetalArgumentBufferManager::Free(MetalArgumentBufferHandle handle)
    {
        CHECK_FORMAT(std::ranges::find(mFreedIndices, handle.index) == mFreedIndices.end(), "Freed the argument buffer handle that already was freed.");

        mFreedIndices.push_back(handle.index);
    }
} // namespace cube
