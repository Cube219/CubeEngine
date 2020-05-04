#pragma once

#include "Utility/Types.h"
#include "../Assertion.h"

namespace cube
{
    template <typename T>
    T Align(T offset, T alignment)
    {
        CHECK((alignment & (alignment - 1)) == 0, "Alignment({0}) must be power of 2.", alignment);
        return (offset + (alignment - 1)) & ~(alignment - 1);
    }
} // namespace cube
