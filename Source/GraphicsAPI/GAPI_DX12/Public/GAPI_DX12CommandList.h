#pragma once

#include "DX12Header.h"

#include "GAPI_CommandList.h"

namespace cube
{
    class DX12Device;

    namespace gapi
    {
        class DX12CommandList : public CommandList
        {
        public:
            DX12CommandList(const CommandListCreateInfo& info);
            virtual ~DX12CommandList();
        };
    } // namespace gapi
} // namespace cube
