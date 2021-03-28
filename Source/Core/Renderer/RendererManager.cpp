#include "RendererManager.h"

#include "../Assertion.h"

#include "Platform/Platform.h"
#include "RenderAPIs/RenderAPI/Interface/Framebuffer.h"
#include "RenderAPIs/RenderAPI/Interface/PipelineState.h"
#include "RenderAPIs/RenderAPI/Interface/RenderPass.h"
#include "RenderAPIs/RenderAPI/Interface/Sampler.h"
#include "RenderAPIs/RenderAPI/Interface/ShaderVariables.h"
#include "RenderAPIs/RenderAPI/Interface/SwapChain.h"
#include "RenderAPIs/RenderAPI/Interface/Texture.h"
#include "RenderAPIs/RenderAPI/Interface/TextureView.h"

namespace cube
{
    SPtr<platform::DLib> RendererManager::mRenderAPIDLib;
    SPtr<rapi::RenderAPI> RendererManager::mRenderAPI;
    bool RendererManager::mVsync;
    Uint32 RendererManager::mWidth;
    Uint32 RendererManager::mHeight;
    Uint32 RendererManager::mSwapChainBufferCount;
    rapi::TextureFormat RendererManager::mColorTextureFormat;
    rapi::TextureFormat RendererManager::mDepthTextureFormat;

    HandlerTable RendererManager::mRenderObjectTable(50);
    Vector<UPtr<RenderObject>> RendererManager::mRenderObjects;

    SPtr<rapi::Sampler> RendererManager::mDefaultSampler;
    SPtr<rapi::SwapChain> RendererManager::mSwapChain;
    SPtr<rapi::RenderPass> RendererManager::mRenderPass;
    Array<SPtr<rapi::Framebuffer>, MaxSwapChainBufferCount> RendererManager::mFramebuffers;
    Array<SPtr<rapi::Texture>, MaxSwapChainBufferCount> RendererManager::mColorTextures;
    Array<SPtr<rapi::TextureView>, MaxSwapChainBufferCount> RendererManager::mColorTextureViews;
    Array<SPtr<rapi::Texture>, MaxSwapChainBufferCount> RendererManager::mDepthTextures;
    Array<SPtr<rapi::TextureView>, MaxSwapChainBufferCount> RendererManager::mDepthTextureViews;

    void RendererManager::Initialize(const RendererManagerInitInfo& initInfo)
    {
        RendererManagerRT::Initialize(initInfo);
    }

    void RendererManager::Shutdown()
    {
        RendererManagerRT::Shutdown();
    }

    HMaterial RendererManager::RegisterMaterial(UPtr<Material>&& material)
    {
        HMaterial mat = RegisterRenderObject(std::move(material));

        RenderingThread::QueueTask([rtMat = DPCast(rt::Material)(mat->GetRTObject())]() {
            RendererManagerRT::AddMaterial(rtMat);
        });

        return mat;
    }

    UPtr<Material> RendererManager::UnregisterMaterial(HMaterial& material)
    {
        RenderingThread::QueueTask([rtMat = DPCast(rt::Material)(material->GetRTObject())]() {
            RendererManagerRT::RemoveMaterial(rtMat);
        });

        return UnregisterRenderObject(material);
    }

    void RendererManager::Resize(Uint32 width, Uint32 height)
    {
        RenderingThread::QueueSyncTask([width, height]() {
            RendererManagerRT::Resize(width, height);
        });
    }

    // --------- RendererManagerRT ---------

    void RendererManagerRT::Initialize(const RendererManagerInitInfo& initInfo)
    {
        switch(initInfo.apiType)
        {
            case RenderAPIType::Vulkan:
                mRenderAPIDLib = platform::Platform::LoadDLib(CUBE_T("CE-VulkanAPI"));
                break;

            default:
                ASSERTION_FAILED("Unknown render api type. ({})", (int)initInfo.apiType);
                break;
        }

        using CreateRenderAPIFunction = rapi::RenderAPI* (*)();
        auto createRenderAPIFunc = RCast(CreateRenderAPIFunction)(mRenderAPIDLib->GetFunction(CUBE_T("CreateRenderAPI")));
        mRenderAPI = SPtr<rapi::RenderAPI>(createRenderAPIFunc());

        rapi::RenderAPICreateInfo apiCreateInfo; // TODO: Multi-thread 구현시 CommandPool 개수 넣기
        mRenderAPI->Initialize(apiCreateInfo);

        // Create default sampler
        rapi::SamplerCreateInfo samplerCreateInfo; // TODO: default sampler 설정 넣기
        mDefaultSampler = mRenderAPI->CreateSampler(samplerCreateInfo);

        // TODO: 사용자가 수정 가능하도록
        mSwapChainBufferCount = 2;
        mColorTextureFormat = rapi::TextureFormat::RGBA_8_UNorm;
        mDepthTextureFormat = rapi::TextureFormat::D16_UNorm;
        mVsync = false;

        CreateSwapChain();
        CreateRenderPass();
        CreateFramebuffer();
        CreateShaderVariables();
    }

    void RendererManagerRT::Shutdown()
    {
        mPerObjectShaderVariablesLayout = nullptr;
        mGlobalShaderVariables = nullptr;
        mGlobalShaderVariablesLayout = nullptr;

        mDepthTextureViews.fill(nullptr);
        mDepthTextures.fill(nullptr);
        mColorTextureViews.fill(nullptr);
        mColorTextureViews.fill(nullptr);
        mFramebuffers.fill(nullptr);
        mSwapChain = nullptr;
        mRenderPass = nullptr;

        mDefaultSampler = nullptr;

        mRenderAPI = nullptr;
        mRenderAPIDLib = nullptr;
    }

    void RendererManagerRT::Render()
    {
        
    }

    void RendererManagerRT::Resize(Uint32 width, Uint32 height)
    {
        mWidth = width;
        mHeight = height;
        // TODO: 이미 그려지고 있는 것과 안 겹치게 수정
    }

    void RendererManagerRT::AddMaterial(SPtr<rt::Material> rtMaterial)
    {
        rtMaterial->mMaterialIndex = SCast(Int32)(mMaterials.size());
        mMaterials.push_back(rtMaterial);
        mMaterialPipelines.push_back(CreatePipeline(rtMaterial));
    }

    void RendererManagerRT::RemoveMaterial(SPtr<rt::Material> rtMaterial)
    {
        Int32 index = rtMaterial->mMaterialIndex;
        Int32 lastIndex = mMaterials.back()->mMaterialIndex;
        mMaterials[lastIndex]->mMaterialIndex = index;
        mMaterials[index] = nullptr;

        std::swap(mMaterials[index], mMaterials[lastIndex]);
        std::swap(mMaterialPipelines[index], mMaterialPipelines[lastIndex]);
        mMaterials.pop_back();
        mMaterialPipelines.pop_back();

        rtMaterial->mMaterialIndex = -1;
    }

    SPtr<rapi::GraphicsPipelineState> RendererManagerRT::CreatePipeline(SPtr<rt::Material>& rtMaterial)
    {
        rapi::GraphicsPipelineStateCreateInfo pipelineCreateInfo;
    }

    void RendererManagerRT::CreateSwapChain()
    {
        mSwapChain = nullptr;

        rapi::SwapChainCreateInfo swapChainCreateInfo;
        swapChainCreateInfo.width = mWidth;
        swapChainCreateInfo.height = mHeight;
        swapChainCreateInfo.colorBufferFormat = mColorTextureFormat;
        swapChainCreateInfo.depthBufferFormat = mDepthTextureFormat;
        swapChainCreateInfo.vsync = mVsync;
        swapChainCreateInfo.bufferCount = mSwapChainBufferCount;
        mSwapChain = mRenderAPI->CreateSwapChain(swapChainCreateInfo);
    }

    void RendererManagerRT::CreateRenderPass()
    {
        mRenderPass = nullptr;

        FrameVector<rapi::RenderPassAttachmentInfo> attachments(2);

        // Color attachment
        rapi::RenderPassAttachmentInfo& colorAttachment = attachments[0];
        colorAttachment.format = mColorTextureFormat;
        colorAttachment.sampleCount = 1;
        colorAttachment.loadOp = rapi::LoadOperator::Clear;
        colorAttachment.storeOp = rapi::StoreOperator::Store;
        colorAttachment.stencilLoadOp = rapi::LoadOperator::DontCare;
        colorAttachment.stencilStoreOp = rapi::StoreOperator::DontCare;

        // Depth attachment
        rapi::RenderPassAttachmentInfo& depthAttachment = attachments[1];
        depthAttachment.format = mDepthTextureFormat;
        depthAttachment.sampleCount = 1;
        depthAttachment.loadOp = rapi::LoadOperator::Clear;
        depthAttachment.storeOp = rapi::StoreOperator::DontCare;
        depthAttachment.stencilLoadOp = rapi::LoadOperator::DontCare;
        depthAttachment.stencilStoreOp = rapi::StoreOperator::DontCare;

        // Subpass
        FrameVector<rapi::Subpass> subpasses(1);

        rapi::Subpass& mainSubpass = subpasses[0];
        Uint32 colorAttachmentIndex = 0;
        mainSubpass.numColorAttachments = 1;
        mainSubpass.colorAttachmentIndices = &colorAttachmentIndex;
        mainSubpass.depthStencilAttachmentIndex = 1;

        rapi::RenderPassCreateInfo renderPassCreateInfo;
        renderPassCreateInfo.numAttachments = SCast(Uint32)(attachments.size());
        renderPassCreateInfo.attachments = attachments.data();
        renderPassCreateInfo.numSubpasses = SCast(Uint32)(subpasses.size());
        renderPassCreateInfo.subpasses = subpasses.data();
        mRenderPass = mRenderAPI->CreateRenderPass(renderPassCreateInfo);
    }

    void RendererManagerRT::CreateFramebuffer()
    {
        mFramebuffers.fill(nullptr);
        mColorTextureViews.fill(nullptr);
        mColorTextures.fill(nullptr);
        mDepthTextureViews.fill(nullptr);
        mDepthTextures.fill(nullptr);

        // Color texture
        rapi::TextureCreateInfo textureCreateInfo;
        textureCreateInfo.dimension = rapi::TextureDimension::Texture2D;
        textureCreateInfo.width = mWidth;
        textureCreateInfo.height = mHeight;
        textureCreateInfo.depth = 1;
        textureCreateInfo.arraySize = 1;
        textureCreateInfo.data = FrameVector<rapi::TextureData>();
        textureCreateInfo.format = mColorTextureFormat;
        textureCreateInfo.bindTypeFlags = rapi::TextureBindTypeFlag::RenderTarget;
        textureCreateInfo.generateMipmaps = false;
        textureCreateInfo.samplesNum = 1;
        for(Uint32 i = 0; i < mSwapChainBufferCount; ++i) {
            mColorTextures[i] = mRenderAPI->CreateTexture(textureCreateInfo);
            mColorTextureViews[i] = mColorTextures[i]->CreateDefaultView();
        }

        // Depth texture
        textureCreateInfo.format = mDepthTextureFormat;
        textureCreateInfo.bindTypeFlags = rapi::TextureBindTypeFlag::DepthStencil;
        for(Uint32 i = 0; i < mSwapChainBufferCount; ++i) {
            mDepthTextures[i] = mRenderAPI->CreateTexture(textureCreateInfo);
            mDepthTextureViews[i] = mDepthTextures[i]->CreateDefaultView();
        }

        rapi::FramebufferCreateInfo framebufferCreateInfo;
        framebufferCreateInfo.pRenderPass = mRenderPass.get();
        framebufferCreateInfo.numAttachments = 2;
        framebufferCreateInfo.ppAttachments = nullptr;
        framebufferCreateInfo.width = mWidth;
        framebufferCreateInfo.height = mHeight;
        framebufferCreateInfo.arrayStartIndex = 0;
        framebufferCreateInfo.debugName = "Main Framebuffer";
        FrameVector<rapi::TextureView*> attachments(2);
        for(Uint32 i = 0; i < mSwapChainBufferCount; ++i) {
            attachments[0] = mColorTextureViews[i].get();
            attachments[1] = mDepthTextureViews[i].get();
            framebufferCreateInfo.ppAttachments = attachments.data();
            mFramebuffers[i] = mRenderAPI->CreateFramebuffer(framebufferCreateInfo);
        }
    }

    void RendererManagerRT::CreateShaderVariables()
    {
        rapi::ShaderVariableInfo vInfo;

        rapi::ShaderVariablesLayoutCreateInfo globalLayoutCreateInfo;
        vInfo.type = rapi::ShaderVariableType::Constant;
        vInfo.size = sizeof(GlobalShaderData);
        vInfo.count = 1;
        globalLayoutCreateInfo.variableInfos.push_back(vInfo);
        globalLayoutCreateInfo.debugName = "Global shader variables layout";
        mGlobalShaderVariablesLayout = mRenderAPI->CreateShaderVariablesLayout(globalLayoutCreateInfo);
        mGlobalShaderVariables = mGlobalShaderVariablesLayout->CreateVariables();

        rapi::ShaderVariablesLayoutCreateInfo perObjectLayoutCreateInfo;
        vInfo.type = rapi::ShaderVariableType::Constant;
        vInfo.size = sizeof(PerObjectShaderData);
        vInfo.count = 1;
        perObjectLayoutCreateInfo.variableInfos.push_back(vInfo);
        perObjectLayoutCreateInfo.debugName = "Per-object shader variables layout";
        mPerObjectShaderVariablesLayout = mRenderAPI->CreateShaderVariablesLayout(perObjectLayoutCreateInfo);
    }
} // namespace cube
