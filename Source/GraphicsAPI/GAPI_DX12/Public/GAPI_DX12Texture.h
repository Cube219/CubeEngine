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
            virtual SharedPtr<TextureRTV> CreateRTV(const TextureRTVCreateInfo& createInfo) override;
            virtual SharedPtr<TextureDSV> CreateDSV(const TextureDSVCreateInfo& createInfo) override;

            ID3D12Resource* GetResource() const { return mResource; }

        protected:
            // From existing resource (ex: swapchain backbuffer)
            DX12Texture(const TextureCreateInfo& info, ID3D12Resource* resource, DX12Device& device);

            DX12Device& mDevice;

            DX12Allocation mAllocation;
            ID3D12Resource* mResource;
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
            D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle() const { return mSRVDescriptor.handle; }

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
            D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle() const { return mUAVDescriptor.handle; }

        private:
            DX12Device& mDevice;

            DX12DescriptorHandle mUAVDescriptor;
        };

        class DX12TextureRTV : public TextureRTV, public DX12APIObject
        {
        public:
            DX12TextureRTV(const TextureRTVCreateInfo& createInfo, SharedPtr<Texture> texture, DX12Device& device);
            virtual ~DX12TextureRTV();

            DX12Texture* GetDX12Texture() const { return dynamic_cast<DX12Texture*>(mTexture.get()); }
            D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle() const { return mRTVDescriptor.handle; }

        private:
            DX12Device& mDevice;

            DX12DescriptorHandle mRTVDescriptor;
        };

        class DX12TextureDSV : public TextureDSV, public DX12APIObject
        {
        public:
            DX12TextureDSV(const TextureDSVCreateInfo& createInfo, SharedPtr<Texture> texture, DX12Device& device);
            virtual ~DX12TextureDSV();

            DX12Texture* GetDX12Texture() const { return dynamic_cast<DX12Texture*>(mTexture.get()); }
            D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle() const { return mDSVDescriptor.handle; }

        private:
            DX12Device& mDevice;

            DX12DescriptorHandle mDSVDescriptor;
        };
    } // namespace gapi
} // namespace cube
