#pragma once

#include "CubeString.h"
#include "GAPIHeader.h"

namespace cube
{
    namespace gapi
    {
        struct FenceCreateInfo
        {
            bool allowCPUAccess = false;

            StringView debugName;
        };

        class Fence
        {
        public:
            Fence(const FenceCreateInfo& createInfo)
                : mAllowCPUAccess(createInfo.allowCPUAccess)
                , mDebugName(createInfo.debugName)
            {}
            virtual ~Fence() = default;

            virtual void Wait(Uint64 fenceValue) = 0;
            virtual void Signal(Uint64 fenceValue) = 0;

            virtual Uint64 GetCompletedValue() = 0;

        protected:
            bool mAllowCPUAccess;

            String mDebugName;
        };
    } // namespace gapi
} // namespace cube
