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

#define CUBE_DX12_BOUND_OBJECT(object) mBoundObjects.push_back(std::dynamic_pointer_cast<DX12APIObject>(object))
} // namespace cube
