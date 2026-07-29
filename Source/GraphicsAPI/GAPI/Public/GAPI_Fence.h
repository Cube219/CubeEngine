#pragma once

#include "CubeString.h"
#include "GAPIHeader.h"

namespace cube
{
    namespace gapi
    {
        struct FenceCreateInfo
        {
            StringView debugName;
        };

        class Fence
        {
        public:
            Fence() = default;
            virtual ~Fence() = default;

            virtual void Wait(Uint64 fenceValue) = 0;
            virtual Uint64 GetCompletedValue() = 0;
        };
    } // namespace gapi
} // namespace cube
