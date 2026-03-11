#include "TextureManager.h"

#include "Allocator/FrameAllocator.h"
#include "Checker.h"
#include "Engine.h"
#include "FileSystem.h"
#include "GAPI.h"
#include "GAPI_CommandList.h"
#include "GAPI_Pipeline.h"
#include "RenderGraph.h"
#include "Renderer.h"
#include "Shader.h"
#include "Texture.h"

namespace cube
{
    void TextureManager::Initialize(GAPI* gapi, Uint32 numGPUSync, ShaderManager& shaderManager)
    {
        mGAPI = gapi;

        {
            platform::FilePath shaderFilePath = Engine::GetShaderDirectoryPath() / CUBE_T("GenerateMipmaps.slang");
        
            mGenerateMipmapsShader = shaderManager.CreateShader({
                .type = gapi::ShaderType::Compute,
                .language = gapi::ShaderLanguage::Slang,
                .filePaths = { &shaderFilePath, 1 },
                .entryPoint = "CSMain",
                .debugName = CUBE_T("GenerateMipmapsShaderCS")
            });
            CHECK(mGenerateMipmapsShader);
        
            mGenerateMipmapsPipeline = shaderManager.CreateComputePipeline({
                .shader = mGenerateMipmapsShader,
                .debugName = CUBE_T("GenerateMipmapsComputePipeline")
            });
        }

        mCommandList = mGAPI->CreateCommandList({
            .debugName = CUBE_T("TextureManagerCommandList")
        });
    }

    void TextureManager::Shutdown()
    {
        mCommandList = nullptr;

        mGenerateMipmapsPipeline = nullptr;
        mGenerateMipmapsShader = nullptr;
    }

    void TextureManager::GenerateMipmaps(SharedPtr<gapi::Texture> texture)
    {
        // TODO: Implement other types
        if (texture->GetType() != gapi::TextureType::Texture2D)
        {
            NOT_IMPLEMENTED();
            return;
        }

        RGBuilder builder;
        {
            RG_GPU_EVENT_SCOPE(builder, CUBE_T("GenerateMipmaps"));

            Uint32 width = texture->GetWidth();
            Uint32 height = texture->GetHeight();
            Uint32 mipLevels = texture->GetMipLevels();

            RGTexture* rgTexture = builder.RegisterTexture(texture);

            for (Uint32 mipIndex = 1; mipIndex < mipLevels; ++mipIndex)
            {
                width = std::max(1u, width >> 1);
                height = std::max(1u, height >> 1);

                RGTextureSRV* srcSRV = builder.CreateSRV(rgTexture, mipIndex - 1);
                RGTextureUAV* dstUAV = builder.CreateUAV(rgTexture, mipIndex);

                builder.AddPass(Format<FrameString>(CUBE_T("GenerateMipmaps ({0}->{1})"), mipIndex - 1, mipIndex),
                [pipeline = mGenerateMipmapsPipeline, width, height, srcSRV, dstUAV](gapi::CommandList& commandList)
                {
                    ShaderParametersManager& shaderParametersManager = Engine::GetRenderer()->GetShaderParametersManager();
                    SharedPtr<GenerateMipmapsShaderParameters> parameters = shaderParametersManager.CreateShaderParameters<GenerateMipmapsShaderParameters>();

                    SharedPtr<gapi::TextureSRV> src = srcSRV->GetSRV();
                    SharedPtr<gapi::TextureUAV> dst = dstUAV->GetUAV();

                    parameters->srcTexture.id = src->GetBindlessId();
                    parameters->dstTexture.id = dst->GetBindlessId();
                    parameters->WriteAllParametersToBuffer();

                    commandList.SetComputePipeline(pipeline->GetGAPIComputePipeline());

                    // TODO: Move to RenderGraph in the future. Currently UseResource should be called
                    // after set compute piepline.
                    commandList.UseResource(src);
                    commandList.UseResource(dst);

                    commandList.SetShaderVariableConstantBuffer(0, parameters->GetBuffer());

                    commandList.DispatchThreads(width, height, 1);
                },
                [srcSRV, dstUAV](RGBuilder& builder)
                {
                    builder.UseResource(srcSRV);
                    builder.UseResource(dstUAV);
                });
            }
        }

        builder.ExecuteAndSubmit(*mCommandList);
    }
} // namespace cube
