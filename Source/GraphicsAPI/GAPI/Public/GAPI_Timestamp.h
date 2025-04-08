#pragma once

#include "GAPIHeader.h"

#include "CubeString.h"

namespace cube
{
    namespace gapi
    {
        struct Timestamp
        {
            String name;
            Uint64 time;
        };

        struct TimestampList
        {
            Uint64 frame = 0;
            Uint64 frequency;

            Vector<Timestamp> timestamps;
        };
    } // namespace gapi
} // namespace cube
