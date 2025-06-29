#pragma once

#include "DX12Header.h"

#include "GAPI_Fence.h"

namespace cube
{
    class DX12Device;

    namespace gapi
    {
        class DX12Fence_old : public Fence
        {
        public:
            DX12Fence_old(const FenceCreateInfo& info);
            virtual ~DX12Fence_old();
        };
    } // namespace gapi
} // namespace cube
