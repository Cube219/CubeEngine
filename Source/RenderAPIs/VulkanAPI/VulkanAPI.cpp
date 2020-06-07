#include "VulkanAPI.h"

#include "VulkanUtility.h"
#include "Core/Assertion.h"
#include "VulkanTypeConversion.h"
#include "VulkanDebug.h"
#include "Interface/TextureVk.h"

namespace cube
{
    namespace rapi
    {
        RenderAPI* CreateRenderAPI()
        {
            return new VulkanAPI();
        }

        void VulkanAPI::Initialize(bool enableDebugLayer)
        {
            InitTypeConversion();

            CreateInstance(enableDebugLayer);
            if(enableDebugLayer == true) {
                VULKAN_DEBUG_INIT(mInstance);
            }
            GetDevices();
        }

        void VulkanAPI::Shutdown()
        {
            VULKAN_DEBUG_SHUTDOWN();
        }

        SPtr<Texture2D> VulkanAPI::CreateTexture2D(const Texture2DCreateInfo& info)
        {
            return std::make_shared<Texture2DVk>(*mDevice, info);
        }

        SPtr<Texture2DArray> VulkanAPI::CreateTexture2DArray(const Texture2DArrayCreateInfo& info)
        {
            return std::make_shared<Texture2DArrayVk>(*mDevice, info);
        }

        SPtr<Texture3D> VulkanAPI::CreateTexture3D(const Texture3DCreateInfo& info)
        {
            return std::make_shared<Texture3DVk>(*mDevice, info);
        }

        SPtr<TextureCube> VulkanAPI::CreateTextureCube(const TextureCubeCreateInfo& info)
        {
            return std::make_shared<TextureCubeVk>(*mDevice, info);
        }

        void VulkanAPI::CreateInstance(bool enableDebugLayer)
        {
            VkResult res;

            // Create instance
            FrameVector<const char*> layers;
            FrameVector<const char*> extensions;
            extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef VK_USE_PLATFORM_WIN32_KHR
            extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif // VK_USE_PLATFORM_WIN32_KHR
            if(enableDebugLayer == true) {
                layers.push_back("VK_LAYER_KHRONOS_validation");
            }

            VkApplicationInfo appInfo;
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pNext = nullptr;
            appInfo.pApplicationName = nullptr;
            appInfo.applicationVersion = 0;
            appInfo.pEngineName = "CubeEngine";
            appInfo.engineVersion = 0;
            appInfo.apiVersion = VK_API_VERSION_1_2;

            VkInstanceCreateInfo instanceCreateInfo = {};
            instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            instanceCreateInfo.pNext = nullptr;
            instanceCreateInfo.flags = 0;
            instanceCreateInfo.pApplicationInfo = &appInfo;
            instanceCreateInfo.enabledExtensionCount = SCast(Uint32)(extensions.size());
            instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
            instanceCreateInfo.enabledLayerCount = SCast(Uint32)(layers.size());
            instanceCreateInfo.ppEnabledLayerNames = layers.data();

            res = vkCreateInstance(&instanceCreateInfo, nullptr, &mInstance);
            CHECK_VK(res, "Failed to create VkInstance.");
        }

        void VulkanAPI::GetDevices()
        {
            VkResult res;

            Uint32 deviceCount = 0;
            res = vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);
            CHECK_VK(res, "Failed to enumberate VulkanDevice.");
            CHECK(deviceCount > 0, "Cannot find GPUs that support Vulkan.");

            FrameVector<VkPhysicalDevice> physicalDevices;
            physicalDevices.resize(deviceCount);
            res = vkEnumeratePhysicalDevices(mInstance, &deviceCount, physicalDevices.data());
            CHECK_VK(res, "Failed to enumberate VulkanDevice.");

            mAllDevices.reserve(deviceCount);
            for(Uint32 i = 0; i < deviceCount; i++) {
                mAllDevices.emplace_back(mInstance, physicalDevices[i]);
            }

            mDevice = nullptr;
            for(Uint32 i = 0; i < deviceCount; i++) {
                if(mAllDevices[i].GetGPUType() == GPUType::Discrete) {
                    mDevice = &mAllDevices[i];
                    break;
                }
            }
            if(mDevice == nullptr) mDevice = &mAllDevices[0];
        }
    } // namespace rapi
} // namespace cube
