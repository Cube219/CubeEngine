#pragma once

#include "Types.h"

namespace cube
{
    class IAllocator
    {
    public:
        virtual ~IAllocator() = default;

        virtual void* Allocate(SizeType n) = 0;
        virtual void Free(void* ptr, SizeType n) = 0;
    };
} // namespace cube
