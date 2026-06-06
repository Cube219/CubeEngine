#include "RenderUtils.h"

#include "Allocator/FrameAllocator.h"
#include "Engine.h"
#include "Renderer.h"
#include "RenderGraph.h"

namespace cube
{
    class CopyTexturePSParameterList : public ShaderParameterList
    {
        CUBE_BEGIN_SHADER_PARAMETER_LIST(CopyTexturePSParameterList)
            CUBE_SHADER_PARAMETER(RGTextureSRVHandle, srcTexture)
            CUBE_SHADER_PARAMETER(Int2, size)
        CUBE_END_SHADER_PARAMETER_LIST
    };
    CUBE_REGISTER_SHADER_PARAMETER_LIST(CopyTexturePSParameterList);

    RenderUtils::RenderUtils(Renderer& renderer)
        : mRenderer(renderer)
    {}

    void RenderUtils::Initialize()
    {
        platform::FilePath fullScreenVSFilePath = Engine::GetShaderDirectoryPath() / CUBE_T("FullScreenVS.slang");
        mFullScreenVS = mRenderer.GetShaderManager().CreateShader({
            .shaderInfo = {
                .type = gapi::ShaderType::Vertex,
                .language = gapi::ShaderLanguage::Slang,
                .entryPoint = "FullScreenVS"
            },
            .filePaths = { &fullScreenVSFilePath, 1 },
            .debugName = CUBE_T("FullScreenVS")
        });
        CHECK(mFullScreenVS);

        platform::FilePath copyTexturePSFilePath = Engine::GetShaderDirectoryPath() / CUBE_T("CopyTexturePS.slang");
        mCopyTexturePS = mRenderer.GetShaderManager().CreateShader({
            .shaderInfo = {
                .type = gapi::ShaderType::Pixel,
                .language = gapi::ShaderLanguage::Slang,
                .entryPoint = "CopyTexturePS"
            },
            .filePaths = { &copyTexturePSFilePath, 1 },
            .debugName = CUBE_T("CopyTexturePS")
        });
        CHECK(mCopyTexturePS);

        mCopyTexturePSPipelineInfo = {
            .vertexShader = mFullScreenVS,
            .pixelShader = mCopyTexturePS,
        };
    }

    void RenderUtils::Shutdown()
    {
        mCopyTexturePSPipelineInfo = {};
        mCopyTexturePS = nullptr;

        mFullScreenVS = nullptr;
    }

    void RenderUtils::CopyTexturePS(RGBuilder& builder, RGTextureHandle src, RGTextureHandle dst)
    {
        const gapi::TextureInfo& srcInfo = src->GetTextureInfo();
        const gapi::TextureInfo& dstInfo = dst->GetTextureInfo();
        CHECK(srcInfo.width == dstInfo.width);
        CHECK(srcInfo.height == dstInfo.height);

        RGTextureSRVHandle srcSRV = builder.CreateSRV(src);
        RGTextureRTVHandle dstRTV = builder.CreateRTV(dst);

        auto params = builder.CreateShaderParameterList<CopyTexturePSParameterList>();
        params->Get()->srcTexture = srcSRV;
        params->Get()->size = Int2(srcInfo.width, srcInfo.height);

        RGBuilder::RenderPassInfo renderPassInfo;
        renderPassInfo.colors.push_back({
            .color = dstRTV,
            .loadOperation = gapi::LoadOperation::DontCare,
            .storeOperation = gapi::StoreOperation::Store,
        });

        builder.BeginRenderPass(renderPassInfo);

        builder.SetRenderTargetFormatsFromCurrentRenderPass(mCopyTexturePSPipelineInfo);
        SharedPtr<GraphicsPipeline> copyTexturePSPipeline = mRenderer.GetPipelineManager().GetOrCreateGraphicsPipeline({
            .pipelineInfo = mCopyTexturePSPipelineInfo,
            .debugName = CUBE_T("CopyTexturePS Pipeline")
        });

        builder.AddPass(Format<FrameString>(CUBE_T("CopyTexturePS ({0} -> {1})"), src->GetDebugName(), dst->GetDebugName()),
            copyTexturePSPipeline,
            params,
            [](gapi::CommandList& commandList){
                commandList.Draw(3, 0);
            });

        builder.EndRenderPass();
    }
} // namespace cube
