#pragma once

#include "MetalHeader.h"

#include "GAPI_Fence.h"

namespace cube
{
    namespace gapi
    {
        class MetalFence_old : public Fence
        {
        public:
            MetalFence_old(const FenceCreateInfo& info) {}
            virtual ~MetalFence_old() {}
        };
    } // namespace gapi
} // namespace cube
