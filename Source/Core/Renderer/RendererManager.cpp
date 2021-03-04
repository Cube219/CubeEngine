#include "RendererManager.h"

#include "../Assertion.h"
#include "Platform/Platform.h"

namespace cube
{
    SPtr<platform::DLib> RendererManager::mRenderAPIDLib;
    SPtr<rapi::RenderAPI> RendererManager::mRenderAPI;

    void RendererManager::Initialize(RenderAPIType apiType)
    {
        switch(apiType)
        {
            case RenderAPIType::Vulkan:
                mRenderAPIDLib = platform::Platform::LoadDLib(CUBE_T("VulkanAPI"));
                break;

            default:
                ASSERTION_FAILED("Unknown render api type. ({})", (int)apiType);
                break;
        }

        using CreateRenderAPIFunction = rapi::RenderAPI*(*)();
        auto createRenderAPIFunc = RCast(CreateRenderAPIFunction)(mRenderAPIDLib->GetFunction(CUBE_T("CreateRenderAPI")));
        mRenderAPI = SPtr<rapi::RenderAPI>(createRenderAPIFunc());

        rapi::RenderAPICreateInfo apiCreateInfo; // TODO: Multi-thread 구현시 CommandPool 개수 넣기
        mRenderAPI->Initialize(apiCreateInfo);
    }

    void RendererManager::Shutdown()
    {
        mRenderAPI = nullptr;
        mRenderAPIDLib = nullptr;
    }

    void RendererManager::Render()
    {
    }

    void RendererManager::Resize(Uint32 width, Uint32 height)
    {
    }
} // namespace cube
