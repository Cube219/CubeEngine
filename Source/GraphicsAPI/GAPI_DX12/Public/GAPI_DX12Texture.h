#pragma once

#include "DX12Header.h"

#include "GAPI_Texture.h"

#include "DX12APIObject.h"
#include "DX12MemoryAllocator.h"
#include "DX12UploadManager.h"

namespace cube
{
    class DX12Device;

    namespace gapi
    {
        class DX12Texture : public Texture, public DX12APIObject
        {
        public:
            DX12Texture(const TextureCreateInfo& info, DX12Device& device);
            virtual ~DX12Texture();

            virtual void* Map() override;
            virtual void Unmap() override;

            ID3D12Resource* GetResource() const { return mAllocation.allocation->GetResource(); }

        private:
            DX12Device& mDevice;

            DX12Allocation mAllocation;

            D3D12_PLACED_SUBRESOURCE_FOOTPRINT mLayout;
            Uint64 mTotalSize;

            DX12UploadDesc mUploadDesc;
        };
    } // namespace gapi
} // namespace cube
