#include "VulkanAPI.h"

#include "Interface/BufferVk.h"
#include "Interface/FramebufferVk.h"
#include "Interface/PipelineStateVk.h"
#include "Interface/RenderPassVk.h"
#include "Interface/SamplerVk.h"
#include "Interface/ShaderVariablesVk.h"
#include "Interface/ShaderVk.h"
#include "Interface/SwapChainVk.h"
#include "Interface/TextureVk.h"
#include "VulkanUtility.h"
#include "VulkanTypeConversion.h"
#include "VulkanDebug.h"

#include "Core/Assertion.h"

namespace cube
{
    namespace rapi
    {
        RenderAPI* CreateRenderAPI()
        {
            return new VulkanAPI();
        }

        void VulkanAPI::Initialize(const RenderAPICreateInfo& info)
        {
            InitTypeConversion();
            CreateInstance(info.enableDebugLayer);
            if(info.enableDebugLayer == true) {
                VULKAN_DEBUG_INIT(mInstance);
            }
            GetDevices(info);
        }

        void VulkanAPI::Shutdown()
        {
            mDevice = nullptr;
            for(auto device : mAllDevices) {
                delete device;
            }
            mAllDevices.clear();

            VULKAN_DEBUG_SHUTDOWN();
        }

        SPtr<VertexBuffer> VulkanAPI::CreateVertexBuffer(const VertexBufferCreateInfo& info)
        {
            return std::make_shared<VertexBufferVk>(*mDevice, info);
        }

        SPtr<IndexBuffer> VulkanAPI::CreateIndexBuffer(const IndexBufferCreateInfo& info)
        {
            return std::make_shared<IndexBufferVk>(*mDevice, info);
        }

        SPtr<CommandList> VulkanAPI::AllocateCommandList(const CommandListAllocateInfo& info)
        {
            return mDevice->GetCommandPoolManager().AllocateCommandList(info);
        }

        SPtr<Framebuffer> VulkanAPI::CreateFramebuffer(const FramebufferCreateInfo& info)
        {
            return std::make_shared<FramebufferVk>(*mDevice, info);
        }

        SPtr<GraphicsPipelineState> VulkanAPI::CreateGraphicsPipelineState(const GraphicsPipelineStateCreateInfo& info)
        {
            return std::make_shared<GraphicsPipelineStateVk>(*mDevice, info);
        }

        SPtr<ComputePipelineState> VulkanAPI::CreateComputePipelineState(const ComputePipelineStateCreateInfo& info)
        {
            return std::make_shared<ComputePipelineStateVk>(*mDevice, info);
        }

        SPtr<RenderPass> VulkanAPI::CreateRenderPass(const RenderPassCreateInfo& info)
        {
            return std::make_shared<RenderPassVk>(*mDevice, info);
        }

        SPtr<Sampler> VulkanAPI::CreateSampler(const SamplerCreateInfo& info)
        {
            return std::make_shared<SamplerVk>(*mDevice, info);
        }

        SPtr<Shader> VulkanAPI::CreateShader(const ShaderCreateInfo& info)
        {
            return std::make_shared<ShaderVk>(*mDevice, info);
        }

        SPtr<ShaderVariablesLayout> VulkanAPI::CreateShaderVariablesLayout(const ShaderVariablesLayoutCreateInfo& info)
        {
            return std::make_shared<ShaderVariablesLayoutVk>(*mDevice, info);
        }

        SPtr<SwapChain> VulkanAPI::CreateSwapChain(const SwapChainCreateInfo& info)
        {
            return std::make_shared<SwapChainVk>(*mDevice, mInstance, info);
        }

        SPtr<Texture> VulkanAPI::CreateTexture(const TextureCreateInfo& info)
        {
            return std::make_shared<TextureVk>(*mDevice, info);
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
                extensions.push_back("VK_EXT_debug_utils");
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

        void VulkanAPI::GetDevices(const RenderAPICreateInfo& info)
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
                mAllDevices.push_back(new VulkanDevice(info, mInstance, physicalDevices[i]));
            }

            mDevice = nullptr;
            for(Uint32 i = 0; i < deviceCount; i++) {
                if(mAllDevices[i]->GetGPUType() == GPUType::Discrete) {
                    mDevice = mAllDevices[i];
                    break;
                }
            }
            if(mDevice == nullptr) mDevice = mAllDevices[0];
        }
    } // namespace rapi
} // namespace cube
