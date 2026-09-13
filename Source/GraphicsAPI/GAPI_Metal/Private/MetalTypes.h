#pragma once

#include "MetalHeader.h"

#include "GAPITypes.h"

namespace cube
{
    struct MetalElementFormatInfo
    {
        MTLPixelFormat pixelFormat;
        MTLVertexFormat vertexFormat;
        MTLAttributeFormat attributeFormat;
        // BC format: Bytes per 4x4 block
        Uint8 bytes;
    };

    MetalElementFormatInfo GetMetalElementFormatInfo(gapi::ElementFormat elementFormat);

    void InitializeTypes();
} // namespace cube
