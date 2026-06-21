#pragma once

#include "DX12Header.h"

#include "GAPI_Resource.h"

namespace cube
{
    namespace gapi
    {
        D3D12_BARRIER_SYNC ConvertToDX12ResourceSyncFlags(ResourceSyncFlags syncFlags);
        D3D12_BARRIER_ACCESS ConvertToDX12ResourceAccessFlags(ResourceAccessFlags accessFlags);
        D3D12_BARRIER_LAYOUT ConvertToDX12ResourceLayout(ResourceLayout layout);
    } // namespace gapi
} // namespace cube
