#include "RendererManager.h"

#include "../Assertion.h"

#include "Platform/Platform.h"
#include "RenderAPIs/RenderAPI/Interface/Sampler.h"

namespace cube
{
    SPtr<platform::DLib> RendererManager::mRenderAPIDLib;
    SPtr<rapi::RenderAPI> RendererManager::mRenderAPI;

    HandlerTable RendererManager::mRenderObjectTable(50);
    Vector<UPtr<RenderObject>> RendererManager::mRenderObjects;

    SPtr<rapi::Sampler> RendererManager::mDefaultSampler;

    void RendererManager::Initialize(RenderAPIType apiType)
    {
        switch(apiType)
        {
            case RenderAPIType::Vulkan:
                mRenderAPIDLib = platform::Platform::LoadDLib(CUBE_T("CE-VulkanAPI"));
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

        // Create default sampler
        rapi::SamplerCreateInfo samplerCreateInfo; // TODO: default sampler 설정 넣기
        mDefaultSampler = mRenderAPI->CreateSampler(samplerCreateInfo);
    }

    void RendererManager::Shutdown()
    {
        mDefaultSampler = nullptr;

        mRenderAPI = nullptr;
        mRenderAPIDLib = nullptr;
    }

    HMaterial RendererManager::RegisterMaterial(UPtr<Material>&& material)
    {
        HMaterial mat = RegisterRenderObject(std::move(material));

        return mat;
    }

    UPtr<Material> RendererManager::UnregisterMaterial(HMaterial& material)
    {
        return UnregisterRenderObject(material);
    }

    void RendererManager::Render()
    {
    }

    void RendererManager::Resize(Uint32 width, Uint32 height)
    {
    }
} // namespace cube
