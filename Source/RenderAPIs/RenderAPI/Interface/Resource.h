#pragma once

#include "../RenderAPIHeader.h"

namespace cube
{
    namespace rapi
    {
        struct ResourceCreateInfo
        {
            ResourceUsage usage;
            // bool isDedicated; // TODO
            const char* debugName;
        };

        class Resource
        {
        public:
            Resource(ResourceUsage usage, const char* debugName) :
                mUsage(usage), mDebugName(debugName)
            {}
            virtual ~Resource() {}

        protected:
            ResourceUsage mUsage;
            const char* mDebugName = nullptr;
        };
    } // namespace rapi
} // namespace cube
