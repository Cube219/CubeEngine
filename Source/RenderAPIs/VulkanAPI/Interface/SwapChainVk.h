#pragma once

#include "../VulkanAPIHeader.h"

#include "../VulkanSemaphorePool.h"

#include "RenderAPIs/RenderAPI/Interface/SwapChain.h"

namespace cube
{
    namespace rapi
    {
        class SwapChainVk : public SwapChain
        {
        public:
            SwapChainVk(VulkanDevice& device, VkInstance instance, const SwapChainCreateInfo& info);
            virtual ~SwapChainVk();

            void AcquireNextImage() override;
            void Present() override;
            void Resize(Uint32 width, Uint32 height, bool vsync) override;

        private:
            void CreateSwapChain();
            void GetSwapChainImages();

            void UpdateColorFormat(Uint32 numSupportedFormat, VkSurfaceFormatKHR* supportedFormats);
            void UpdateDepthFormat(Uint32 numSupportedFormat, VkSurfaceFormatKHR* supportedFormats);
            void FindPresentMode(VkPhysicalDevice physicalDevice);

            VulkanDevice& mDevice;
            VkInstance mInstance;

            VkSurfaceKHR mSurface;
            VkSwapchainKHR mSwapChain;
            
            Uint32 mWidth;
            Uint32 mHeight;
            Uint32 mBufferCount;
            bool mVsyncSupport;
            bool mVsync;

            VkSurfaceCapabilitiesKHR mSurfaceCapabilities;
            VkFormat mColorFormat;
            VkFormat mDepthFormat;
            VkColorSpaceKHR mColorSpace;
            VkPresentModeKHR mPresentMode;
            VkPresentModeKHR mVsyncPresentMode;

            Vector<VkImageView> mBackBufferImageViews;
            Uint32 mCurrentBackBufferImageIndex;

            VkQueue mPresentQueue;
        };
    } // namespace rapi
} // namespace cube
