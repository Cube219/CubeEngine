#pragma once

#include "MetalHeader.h"

#include "GAPITypes.h"

namespace cube
{
    struct MetalElementFormatInfo
    {
        MTLPixelFormat pixelFormat;
        // BC format: Bytes per 4x4 block
        Uint8 bytes;
        bool unsupported;
    };

    MetalElementFormatInfo GetMetalElementFormatInfo(gapi::ElementFormat elementFormat);

    void InitializeTypes();
} // namespace cube
