#pragma once

#include "GAPIHeader.h"

namespace cube
{
    namespace gapi
    {
        struct FenceCreateInfo
        {
        };

        class Fence
        {
        public:
            Fence() = default;
            virtual ~Fence() = default;
        };
    } // namespace gapi
} // namespace cube
