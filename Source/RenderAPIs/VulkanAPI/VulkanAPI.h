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

            virtual void Initialize(const RenderAPICreateInfo& info) override;
            virtual void Shutdown() override;

            virtual SPtr<Texture> CreateTexture(const TextureCreateInfo& info) override;

            VkInstance GetInstance() const { return mInstance; }
            VulkanDevice* GetDevice() { return mDevice; }

        private:
            void CreateInstance(bool enableDebugLayer);
            void GetDevices(const RenderAPICreateInfo& info);

            VkInstance mInstance;
            Vector<VulkanDevice*> mAllDevices;
            VulkanDevice* mDevice;
        };
    } // namespace rapi
} // namespace cube
