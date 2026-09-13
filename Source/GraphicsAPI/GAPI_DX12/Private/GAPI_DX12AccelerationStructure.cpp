#include "GAPI_DX12AccelerationStructure.h"

namespace cube
{
    namespace gapi
    {
        DX12BLAS::DX12BLAS(const BLASCreateInfo& createInfo, DX12Device& device)
            : BLAS(createInfo)
            , mDevice(device)
        {
        }

        DX12BLAS::~DX12BLAS()
        {
        }
    } // namespace gapi
} // namespace cube
