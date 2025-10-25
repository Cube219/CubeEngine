#pragma once

#include "MetalHeader.h"

namespace cube
{
    class MetalDevice;

    struct MetalArgumentBufferHandle
    {
        int index = -1;
    };

    class MetalArgumentBufferManager
    {
    public:
        MetalArgumentBufferManager(MetalDevice& device);
        ~MetalArgumentBufferManager();

        MetalArgumentBufferManager(const MetalArgumentBufferManager& other) = delete;
        MetalArgumentBufferManager& operator=(const MetalArgumentBufferManager& rhs) = delete;

        void Initialize();
        void Shutdown();

        MetalArgumentBufferHandle Allocate();
        void Free(MetalArgumentBufferHandle handle);

    private:
        MetalDevice& mDevice;

        Uint32 mTotalIndices;
        Vector<Uint32> mFreedIndices;
    };
} // namespace cube
