#pragma once

#include "GAPIHeader.h"

#include "CubeString.h"

namespace cube
{
    namespace gapi
    {
        struct TimestampRange
        {
            String name;
            Uint64 beginTime;
            Uint64 endTime;
        };

        struct TimestampRangeList
        {
            Uint64 frame = 0;
            Uint64 frequency;

            Vector<TimestampRange> timestampRanges;
        };
    } // namespace gapi
} // namespace cube
