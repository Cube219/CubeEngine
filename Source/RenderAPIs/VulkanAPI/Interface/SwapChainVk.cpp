#include "SwapChainVk.h"

#include "RenderAPIs/VulkanAPI/VulkanDevice.h"
#include "RenderAPIs/VulkanAPI/VulkanDebug.h"
#include "RenderAPIs/VulkanAPI/VulkanTypeConversion.h"

#include "Core/Allocator/FrameAllocator.h"
#include "Core/LogWriter.h"
#include "Platform/Win32/Win32Platform.h"

namespace cube
{
    namespace rapi
    {
        SwapChainVk::SwapChainVk(VulkanDevice& device, VkInstance instance, const SwapChainCreateInfo& info) :
            mDevice(device), mInstance(instance),
            mWidth(info.width), mHeight(info.height), mBufferCount(info.bufferCount), mVsync(info.vsync),
            mSwapChain(VK_NULL_HANDLE),
            mCurrentBackBufferImageIndex(0)
        {
            VkResult res;

            // Create os-dependent surface
#if defined(VK_USE_PLATFORM_WIN32_KHR)
            VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.pNext = nullptr;
            surfaceCreateInfo.flags = 0;
            surfaceCreateInfo.hinstance = platform::Win32Platform::GetInstance();
            surfaceCreateInfo.hwnd = platform::Win32Platform::GetWindow();

            res = vkCreateWin32SurfaceKHR(mInstance, &surfaceCreateInfo, nullptr, &mSurface);
#endif // defined(VK_USE_PLATFORM_WIN32_KHR)
            CHECK_VK(res, "Failed to create surface.");
            VULKAN_SET_OBJ_NAME(mDevice.GetHandle(), mSurface, "Vulkan Swapchain Surface");

            // Get SurfaceCapabilities
            VkPhysicalDevice physicalDevice = mDevice.GetGPU();
            res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, mSurface, &mSurfaceCapabilities);
            CHECK_VK(res, "Failed to get SurfaceCapabilities.");

            // Find color/depth surface format
            FrameVector<VkSurfaceFormatKHR> supportedFormats;
            {
                Uint32 numFormat = 0;
                res = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, mSurface, &numFormat, nullptr);
                CHECK_VK(res, "Failed to get number of supported surface formats.");

                supportedFormats.resize(numFormat);
                res = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, mSurface, &numFormat, supportedFormats.data());
                CHECK_VK(res, "Failed to get supported surface formats.")
            }
            mColorFormat = TextureFormatToVkFormat(info.colorBufferFormat);
            mDepthFormat = TextureFormatToVkFormat(info.depthBufferFormat);
            UpdateColorFormat(SCast(Uint32)(supportedFormats.size()), supportedFormats.data());
            UpdateDepthFormat(SCast(Uint32)(supportedFormats.size()), supportedFormats.data());

            FindPresentMode(physicalDevice);

            mPresentQueue = mDevice.GetQueueManager().GetPresentQueue(mSurface);

            CreateSwapChain();
            GetSwapChainImages();
        }

        SwapChainVk::~SwapChainVk()
        {
            for(auto imageView : mBackBufferImageViews) {
                vkDestroyImageView(mDevice.GetHandle(), imageView, nullptr);
            }
            mBackBufferImageViews.clear();

            vkDestroySwapchainKHR(mDevice.GetHandle(), mSwapChain, nullptr);
            vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
        }

        void SwapChainVk::AcquireNextImage()
        {
            VkResult res;

            VulkanSemaphore signalSemaphore = mDevice.GetSemaphorePool().AllocateSemaphore("Semaphore to signal that acquire back buffer image");

            res = vkAcquireNextImageKHR(mDevice.GetHandle(), mSwapChain, UINT64_MAX, 
                signalSemaphore.GetHandle(), VK_NULL_HANDLE, &mCurrentBackBufferImageIndex);
            CHECK_VK(res, "Failed to acquire next image.");

            mDevice.GetQueueManager().AddWaitSemaphoreForGraphics(signalSemaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        }

        void SwapChainVk::Present()
        {
            VkResult res;

            VkPresentInfoKHR info;
            info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            info.pNext = nullptr;
            info.swapchainCount = 1;
            info.pSwapchains = &mSwapChain;
            info.pImageIndices = &mCurrentBackBufferImageIndex;
            info.waitSemaphoreCount = 0;
            info.pWaitSemaphores = nullptr;
            info.pResults = nullptr;

            res = vkQueuePresentKHR(mPresentQueue, &info);
            CHECK_VK(res, "Failed to present.");
        }

        void SwapChainVk::Resize(Uint32 width, Uint32 height, bool vsync)
        {
            if(vsync == true && mVsyncSupport == false) {
                vsync = false;
            }

            // Release all resources related to swap chain
            for(auto imageView : mBackBufferImageViews) {
                vkDestroyImageView(mDevice.GetHandle(), imageView, nullptr);
            }
            mBackBufferImageViews.clear();

            mWidth = width;
            mHeight = height;
            mVsync = vsync;

            CreateSwapChain();
            GetSwapChainImages();
        }

        void SwapChainVk::CreateSwapChain()
        {
            VkSwapchainKHR oldSwapChain = mSwapChain;
            VULKAN_SET_OBJ_NAME(mDevice.GetHandle(), oldSwapChain, "Vulkan Swapchain - old");

            // Clamp width / height / count
            mWidth = std::min(mWidth, mSurfaceCapabilities.maxImageExtent.width);
            mWidth = std::max(mWidth, mSurfaceCapabilities.minImageExtent.width);
            mHeight = std::min(mHeight, mSurfaceCapabilities.maxImageExtent.height);
            mHeight = std::max(mHeight, mSurfaceCapabilities.minImageExtent.height);
            mBufferCount = std::min(mBufferCount, mSurfaceCapabilities.maxImageCount);
            mBufferCount = std::max(mBufferCount, mSurfaceCapabilities.minImageCount);

            VkSwapchainCreateInfoKHR info = {};
            info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            info.pNext = nullptr;
            info.flags = 0;
            info.surface = mSurface;
            info.minImageCount = mBufferCount;
            info.imageFormat = mColorFormat;
            info.imageColorSpace = mColorSpace;
            info.imageExtent = { mWidth, mHeight };
            info.imageArrayLayers = 1;
            info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            info.queueFamilyIndexCount = 0;
            info.pQueueFamilyIndices = nullptr;
            info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
            info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            info.presentMode = mVsync ? mVsyncPresentMode : mPresentMode;
            info.clipped = VK_TRUE;
            info.oldSwapchain = oldSwapChain;

            VkResult res;
            res = vkCreateSwapchainKHR(mDevice.GetHandle(), &info, nullptr, &mSwapChain);
            CHECK_VK(res, "Failed to create VkSwapChainKHR");
            VULKAN_SET_OBJ_NAME(mDevice.GetHandle(), mSwapChain, "Vulkan Swapchain");

            if(oldSwapChain != VK_NULL_HANDLE) {
                vkDestroySwapchainKHR(mDevice.GetHandle(), oldSwapChain, nullptr);
            }
        }

        void SwapChainVk::GetSwapChainImages()
        {
            VkResult res;

            FrameVector<VkImage> swapChainImages;
            {
                Uint32 numImages = 0;
                res = vkGetSwapchainImagesKHR(mDevice.GetHandle(), mSwapChain, &numImages, nullptr);
                CHECK_VK(res, "Failed to get number of swap chain images.");

                swapChainImages.resize(numImages);
                res = vkGetSwapchainImagesKHR(mDevice.GetHandle(), mSwapChain, &numImages, swapChainImages.data());
                CHECK_VK(res, "Failed to get swap chain images.");
            }

            mBackBufferImageViews.resize(swapChainImages.size());
            for(Uint32 i = 0; i < SCast(Uint32)(swapChainImages.size()); ++i) {
                VkImageViewCreateInfo info = {};
                info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                info.pNext = nullptr;
                info.flags = 0;
                info.image = swapChainImages[i];
                info.viewType = VK_IMAGE_VIEW_TYPE_2D;
                info.format = mColorFormat;
                info.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
                info.subresourceRange.baseMipLevel = 0;
                info.subresourceRange.levelCount = 1;
                info.subresourceRange.baseArrayLayer = 0;
                info.subresourceRange.layerCount = 1;
                info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

                res = vkCreateImageView(mDevice.GetHandle(), &info, nullptr, &mBackBufferImageViews[i]);
                CHECK_VK(res, "Failed to create back buffer image view of swap chain.");
                VULKAN_SET_OBJ_NAME(mDevice.GetHandle(), mBackBufferImageViews[i], FrameFormat("SwapChain BackBufferImageView_{}", i).c_str());
            }

            // TODO: DepthImage도?
        }

        void SwapChainVk::UpdateColorFormat(Uint32 numSupportedFormat, VkSurfaceFormatKHR* supportedFormats)
        {
            bool isFound = false;
            for(Uint32 i = 0; i < numSupportedFormat; ++i) {
                if(mColorFormat == supportedFormats[i].format) {
                    isFound = true;
                    mColorSpace = supportedFormats[i].colorSpace;
                    break;
                }
            }
            if(isFound == true) return;

            // BGBA <-> RGBA
            VkFormat alterFormat = VK_FORMAT_UNDEFINED;
            switch(mColorFormat) {
                case VK_FORMAT_R8G8B8A8_UNORM: alterFormat = VK_FORMAT_B8G8R8A8_UNORM; break;
                case VK_FORMAT_R8G8B8A8_SRGB:  alterFormat = VK_FORMAT_B8G8R8A8_SRGB;  break;
                case VK_FORMAT_B8G8R8A8_UNORM: alterFormat = VK_FORMAT_R8G8B8A8_UNORM; break;
                case VK_FORMAT_B8G8R8A8_SRGB:  alterFormat = VK_FORMAT_R8G8B8A8_SRGB;  break;
            }
            isFound = false;
            for(Uint32 i = 0; i < numSupportedFormat; ++i) {
                if(alterFormat == supportedFormats[i].format) {
                    isFound = true;
                    mColorSpace = supportedFormats[i].colorSpace;
                    break;
                }
            }
            if(isFound == true) {
                CUBE_LOG(LogType::Info, "The color image format '{}' is not supported. It is replaced with '{}'.", mColorFormat, alterFormat);
                mColorFormat = alterFormat;
                return;
            } else {
                CUBE_LOG(LogType::Info, "The color image format '{}' is not supported and it cannot be replaced similar. Just use the first supported format '{}'.",
                    mColorFormat, supportedFormats[0].format);
                mColorFormat = supportedFormats[0].format;
                mColorSpace = supportedFormats[0].colorSpace;
            }
        }

        void SwapChainVk::UpdateDepthFormat(Uint32 numSupportedFormat, VkSurfaceFormatKHR* supportedFormats)
        {
            
        }

        void SwapChainVk::FindPresentMode(VkPhysicalDevice physicalDevice)
        {
            VkResult res;

            FrameVector<VkPresentModeKHR> supportedPresentModes;
            {
                Uint32 numPresentMode = 0;
                res = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, mSurface, &numPresentMode, nullptr);
                CHECK_VK(res, "Failed to get number of supported present modes.");

                supportedPresentModes.resize(numPresentMode);
                res = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, mSurface, &numPresentMode, supportedPresentModes.data());
                CHECK_VK(res, "Failed to get supported present modes.");
            }

            mVsyncPresentMode = VK_PRESENT_MODE_FIFO_KHR;
            mVsyncSupport = true;

            mPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            auto findIt = std::find(supportedPresentModes.cbegin(), supportedPresentModes.cend(), VK_PRESENT_MODE_MAILBOX_KHR);
            if(findIt == supportedPresentModes.cend()) {
                mPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
                findIt = std::find(supportedPresentModes.cbegin(), supportedPresentModes.cend(), VK_PRESENT_MODE_IMMEDIATE_KHR);
                if(findIt == supportedPresentModes.cend()) {
                    // Non-vsync mode is not supported, so vsync becomes default and only option.
                    // Because FIFO mode is guaranteed to always be supported.
                    mVsyncSupport = false;
                    mPresentMode = VK_PRESENT_MODE_FIFO_KHR;
                }
            }
        }
    } // namespace rapi
} // namespace cube
