#pragma once

#include "GAPIHeader.h"

namespace cube
{
    namespace gapi
    {
        struct FenceCreateInfo
        {
            const char* debugName = "Unknown";
        };

        class Fence
        {
        public:
            Fence() = default;
            virtual ~Fence() = default;
        };
    } // namespace gapi
} // namespace cube
