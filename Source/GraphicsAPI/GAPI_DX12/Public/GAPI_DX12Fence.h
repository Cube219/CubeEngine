#pragma once

#include "DX12Header.h"

#include "GAPI_Fence.h"

namespace cube
{
    class DX12Device;

    namespace gapi
    {
        class DX12Fence : public Fence
        {
        public:
            DX12Fence(const FenceCreateInfo& info);
            virtual ~DX12Fence();
        };
    } // namespace gapi
} // namespace cube
