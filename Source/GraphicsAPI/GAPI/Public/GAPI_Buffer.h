#pragma once

#include "GAPIHeader.h"

#include "CubeString.h"

#include "GAPI_Resource.h"

namespace cube
{
    namespace gapi
    {
        struct BufferCreateInfo
        {
            ResourceUsage usage;
            Uint64 size;

            const char* debugName = "Unknown";
        };

        class Buffer
        {
        public:
            Buffer() = default;
            virtual ~Buffer() = default;

            virtual void* Map() = 0;
            virtual void Unmap() = 0;
        };
    } // namespace gapi
} // namespace cube
