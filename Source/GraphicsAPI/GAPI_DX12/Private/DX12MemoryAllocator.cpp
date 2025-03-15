#include "DX12MemoryAllocator.h"

#include "DX12Device.h"
#include "DX12Utility.h"

namespace cube
{
    DX12MemoryAllocator::DX12MemoryAllocator(DX12Device& device) :
        mDevice(device)
    {
    }

    void DX12MemoryAllocator::Initialize()
    {
        D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
        allocatorDesc.pDevice = mDevice.GetDevice();
        allocatorDesc.pAdapter = mDevice.GetAdapter();
        allocatorDesc.Flags = (D3D12MA::ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED | D3D12MA::ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED); // D3D12MA_RECOMMENDED_ALLOCATOR_FLAGS;

        CHECK_HR(D3D12MA::CreateAllocator(&allocatorDesc, &mAllocator));
    }

    void DX12MemoryAllocator::Shutdown()
    {
        mAllocator = nullptr;
    }
} // namespace cube
