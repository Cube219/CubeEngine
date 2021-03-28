#pragma once

#include "../CoreHeader.h"

#include "../Handler.h"
#include "Material.h"

#include "Platform/DLib.h"
#include "RenderAPIs/RenderAPI/RenderAPI.h"
#include "Utility/Matrix.h"
#include "Utility/Vector.h"

namespace cube
{
    static constexpr int MaxSwapChainBufferCount = 3;

    enum class RenderAPIType
    {
        Vulkan
    };

    struct GlobalShaderData
    {
        Vector3 cameraPosition;
        Matrix viewProj;
    };

    struct PerObjectShaderData
    {
        Matrix mvp;
        Matrix modelMatrix;
    };

    struct RendererManagerInitInfo
    {
        RenderAPIType apiType = RenderAPIType::Vulkan;
    };

    class CORE_EXPORT RendererManager
    {
    public:
        RendererManager() = delete;
        ~RendererManager() = delete;

        RendererManager(const RendererManager& other) = delete;
        RendererManager& operator=(const RendererManager& rhs) = delete;

        static void Initialize(const RendererManagerInitInfo& initInfo);
        static void Shutdown();

        static HMaterial RegisterMaterial(UPtr<Material>&& material);
        static UPtr<Material> UnregisterMaterial(HMaterial& material);

        static void Resize(Uint32 width, Uint32 height);
        // TODO: 2중/3중 버퍼딩 설정 구현

    private:
        friend class Material;

        template <typename T>
        static Handler<T> RegisterRenderObject(UPtr<T>&& renderObject)
        {
            mRenderObjects.push_back(std::move(renderObject));
            return mRenderObjectTable.CreateNewHandler(mRenderObjects.back().get());
        }

        template <typename T>
        static UPtr<T> UnregisterRenderObject(Handler<T>& hRenderObject)
        {
            T* pRenderObject = mRenderObjectTable.ReleaseHandler(hRenderObject);

            auto findIter = std::find_if(mRenderObjects.begin(), mRenderObjects.end(),
                [pRenderObject](const UPtr<RenderObject>& element) { return element.get() == pRenderObject; }
            );
            
            UPtr<T> uptrRenderObject(DCast(T*)((*findIter).release()));
            mRenderObjects.erase(findIter);

            return uptrRenderObject;
        }

        static HandlerTable mRenderObjectTable;
        static Vector<UPtr<RenderObject>> mRenderObjects;
    };

    class RendererManagerRT
    {
    public:
        RendererManagerRT() = delete;
        ~RendererManagerRT() = delete;

        RendererManagerRT(const RendererManagerRT& other) = delete;
        RendererManagerRT& operator=(const RendererManagerRT& other) = delete;

        static void Initialize(const RendererManagerInitInfo& initInfo);
        static void Shutdown();

        static rapi::RenderAPI& GetRenderAPI() { return *mRenderAPI; }
        static SPtr<rapi::Sampler> GetDefaultSampler() { return mDefaultSampler; }

        static void Render();

        static void Resize(Uint32 width, Uint32 height);

        static void AddMaterial(SPtr<rt::Material> rtMaterial);
        static void RemoveMaterial(SPtr<rt::Material> rtMaterial);

    private:
        static SPtr<rapi::GraphicsPipelineState> CreatePipeline(SPtr<rt::Material>& rtMaterial);
        static void CreateSwapChain();
        static void CreateRenderPass();
        static void CreateFramebuffer();
        static void CreateShaderVariables();

        static SPtr<platform::DLib> mRenderAPIDLib;
        static SPtr<rapi::RenderAPI> mRenderAPI;

        static bool mVsync;
        static Uint32 mWidth;
        static Uint32 mHeight;
        static Uint32 mSwapChainBufferCount;
        static rapi::TextureFormat mColorTextureFormat;
        static rapi::TextureFormat mDepthTextureFormat;

        static SPtr<rapi::Sampler> mDefaultSampler;

        static SPtr<rapi::SwapChain> mSwapChain;
        static SPtr<rapi::RenderPass> mRenderPass;
        static Array<SPtr<rapi::Framebuffer>, MaxSwapChainBufferCount> mFramebuffers;
        static Array<SPtr<rapi::Texture>, MaxSwapChainBufferCount> mColorTextures;
        static Array<SPtr<rapi::TextureView>, MaxSwapChainBufferCount> mColorTextureViews;
        static Array<SPtr<rapi::Texture>, MaxSwapChainBufferCount> mDepthTextures;
        static Array<SPtr<rapi::TextureView>, MaxSwapChainBufferCount> mDepthTextureViews;

        static Vector<SPtr<rt::Material>> mMaterials;
        static Vector<SPtr<rapi::GraphicsPipelineState>> mMaterialPipelines;

        static SPtr<rapi::ShaderVariablesLayout> mGlobalShaderVariablesLayout;
        static SPtr<rapi::ShaderVariables> mGlobalShaderVariables;
        static SPtr<rapi::ShaderVariablesLayout> mPerObjectShaderVariablesLayout;

        static GlobalShaderData mGlobalShaderData;
    };
} // namespace cube
