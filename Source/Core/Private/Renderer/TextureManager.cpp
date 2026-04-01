#include "TextureManager.h"

#include "Allocator/FrameAllocator.h"
#include "Checker.h"
#include "Engine.h"
#include "FileSystem.h"
#include "GAPI.h"
#include "GAPI_CommandList.h"
#include "GAPI_Pipeline.h"
#include "Renderer/RenderGraph.h"
#include "Renderer/RenderGraphTypes.h"
#include "Renderer.h"
#include "Shader.h"
#include "Texture.h"

namespace cube
{
    CUBE_REGISTER_SHADER_PARAMETER_LIST(GenerateMipmapsShaderParameterList);

    TextureManager::TextureManager(Renderer& renderer)
        : mRenderer(renderer)
    {
    }

    void TextureManager::Initialize(GAPI* gapi, Uint32 numGPUSync)
    {
        mGAPI = gapi;

        {
            platform::FilePath shaderFilePath = Engine::GetShaderDirectoryPath() / CUBE_T("GenerateMipmaps.slang");
        
            mGenerateMipmapsShader = mRenderer.GetShaderManager().CreateShader({
                .type = gapi::ShaderType::Compute,
                .language = gapi::ShaderLanguage::Slang,
                .filePaths = { &shaderFilePath, 1 },
                .entryPoint = "CSMain",
                .debugName = CUBE_T("GenerateMipmapsShaderCS")
            });
            CHECK(mGenerateMipmapsShader);
        
            mGenerateMipmapsPipeline = mRenderer.GetShaderManager().CreateComputePipeline({
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

        RGBuilder builder(mRenderer);
        {
            RG_GPU_EVENT_SCOPE(builder, CUBE_T("GenerateMipmaps"));

            Uint32 width = texture->GetWidth();
            Uint32 height = texture->GetHeight();
            Uint32 mipLevels = texture->GetMipLevels();

            RGTextureHandle rgTexture = builder.RegisterTexture(texture);

            for (Uint32 mipIndex = 1; mipIndex < mipLevels; ++mipIndex)
            {
                width = std::max(1u, width >> 1);
                height = std::max(1u, height >> 1);

                RGTextureSRVHandle srcSRV = builder.CreateSRV(rgTexture, mipIndex - 1, 1);
                RGTextureUAVHandle dstUAV = builder.CreateUAV(rgTexture, mipIndex);

                RGShaderParameterListHandle<GenerateMipmapsShaderParameterList> params = builder.CreateShaderParameterList<GenerateMipmapsShaderParameterList>();
                params->Get()->srcTexture = srcSRV;
                params->Get()->dstTexture = dstUAV;
                params->Get()->WriteAllParametersToGPUBuffer();

                builder.AddPass(Format<FrameString>(CUBE_T("GenerateMipmaps ({0}->{1})"), mipIndex - 1, mipIndex),
                mGenerateMipmapsPipeline,
                params,
                [width, height](gapi::CommandList& commandList)
                {
                    commandList.DispatchThreads(width, height, 1);
                });
            }
        }

        builder.ExecuteAndSubmit(*mCommandList);
    }
} // namespace cube
