#pragma once

#include "DX12Header.h"

#include "GAPI_Texture.h"

#include "DX12APIObject.h"
#include "DX12DescriptorManager.h"
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

            virtual SharedPtr<TextureSRV> CreateSRV(const TextureSRVCreateInfo& createInfo) override;
            virtual SharedPtr<TextureUAV> CreateUAV(const TextureUAVCreateInfo& createInfo) override;

            ID3D12Resource* GetResource() const { return mAllocation.allocation->GetResource(); }

        private:
            DX12Device& mDevice;

            DX12Allocation mAllocation;
            DX12UploadDesc mUploadDesc;

            D3D12_PLACED_SUBRESOURCE_FOOTPRINT mLayout;
            Uint64 mTotalSize;
        };

        class DX12TextureSRV : public TextureSRV, public DX12APIObject
        {
        public:
            DX12TextureSRV(const TextureSRVCreateInfo& createInfo, SharedPtr<Texture> texture, DX12Device& device);
            virtual ~DX12TextureSRV();

            DX12Texture* GetDX12Texture() const { return dynamic_cast<DX12Texture*>(mTexture.get()); }

        private:
            DX12Device& mDevice;

            DX12DescriptorHandle mSRVDescriptor;
        };

        class DX12TextureUAV : public TextureUAV, public DX12APIObject
        {
        public:
            DX12TextureUAV(const TextureUAVCreateInfo& createInfo, SharedPtr<Texture> texture, DX12Device& device);
            virtual ~DX12TextureUAV();

            DX12Texture* GetDX12Texture() const { return dynamic_cast<DX12Texture*>(mTexture.get()); }

        private:
            DX12Device& mDevice;

            DX12DescriptorHandle mUAVDescriptor;
        };
    } // namespace gapi
} // namespace cube
