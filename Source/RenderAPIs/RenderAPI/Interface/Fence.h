#pragma once

#include "../RenderAPIHeader.h"

namespace cube
{
    namespace rapi
    {
        enum class FenceWaitResult
        {
            Success,
            Timeout,
            Error
        };

        struct FenceAllocationInfo
        {
            const char* debugName = "";
        };

        class Fence
        {
        public:
            Fence() = default;
            virtual ~Fence() = default;

            virtual void Release() = 0;

            virtual FenceWaitResult Wait(float timeout) = 0;

            bool IsFinished() { return Wait(0.0f) == FenceWaitResult::Success; }
        };
    } // namespace rapi
} // namespace cube
