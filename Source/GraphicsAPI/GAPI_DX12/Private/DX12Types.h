#pragma once

#include "DX12Header.h"

#include "GAPITypes.h"

namespace cube
{
    struct DX12ElementFormatInfo
    {
        DXGI_FORMAT format;
        // BC format: Bytes per 4x4 block
        Uint8 bytes;
        bool unsupported;
    };
    DX12ElementFormatInfo GetDX12ElementFormatInfo(gapi::ElementFormat elementFormat);

    void InitializeTypes();
} // namespace cube
