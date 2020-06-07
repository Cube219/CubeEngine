#pragma once

#include "VulkanAPIHeader.h"

#include "RenderAPIs/RenderAPI/RenderAPI.h"
#include "VulkanDevice.h"

namespace cube
{
    namespace rapi
    {
        extern "C" VK_API_EXPORT RenderAPI* CreateRenderAPI();

        class VK_API_EXPORT VulkanAPI : public RenderAPI
        {
        public:
            VulkanAPI() {}
            virtual ~VulkanAPI() {}

            virtual void Initialize(bool enableDebugLayer = false) override;
            virtual void Shutdown() override;

            virtual SPtr<Texture2D> CreateTexture2D(const Texture2DCreateInfo& info) override;
            virtual SPtr<Texture2DArray> CreateTexture2DArray(const Texture2DArrayCreateInfo& info) override;
            virtual SPtr<Texture3D> CreateTexture3D(const Texture3DCreateInfo& info) override;
            virtual SPtr<TextureCube> CreateTextureCube(const TextureCubeCreateInfo& info) override;

            VkInstance GetInstance() const { return mInstance; }
            VulkanDevice* GetDevice() { return mDevice; }

        private:
            void CreateInstance(bool enableDebugLayer);
            void GetDevices();

            VkInstance mInstance;
            Vector<VulkanDevice> mAllDevices;
            VulkanDevice* mDevice;
        };
    } // namespace rapi
} // namespace cube
