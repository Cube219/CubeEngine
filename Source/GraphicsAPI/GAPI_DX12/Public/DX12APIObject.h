#pragma once

#include "DX12Header.h"

namespace cube
{
    class DX12APIObject : public std::enable_shared_from_this<DX12APIObject>
    {
    public:
        DX12APIObject() {}
        virtual ~DX12APIObject() {}
    };
} // namespace cube
