#pragma once

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
        };
    } // namespace gapi
} // namespace cube
