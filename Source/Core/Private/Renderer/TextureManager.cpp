#include "TextureManager.h"

#include "Checker.h"
#include "Engine.h"
#include "FileSystem.h"
#include "GAPI.h"
#include "GAPI_CommandList.h"
#include "GAPI_Pipeline.h"
#include "RenderProfiling.h"
#include "Renderer.h"
#include "Shader.h"
#include "Texture.h"
#include "Allocator/FrameAllocator.h"

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

        mCommandList->Reset();
        mCommandList->Begin();

        {
            GPU_EVENT_SCOPE(mCommandList, CUBE_T("GenerateMipmaps"));
            
            mCommandList->SetComputePipeline(mGenerateMipmapsPipeline->GetGAPIComputePipeline());

            Uint32 width = texture->GetWidth();
            Uint32 height = texture->GetHeight();
            Uint32 mipLevels = texture->GetMipLevels();

            FrameVector<SharedPtr<gapi::TextureSRV>> srvs(mipLevels);
            FrameVector<SharedPtr<gapi::TextureUAV>> uavs(mipLevels);
            for (Uint32 i = 0; i < mipLevels; ++i)
            {
                srvs[i] = texture->CreateSRV({
                    .firstMipLevel = i,
                    .mipLevels = 1
                });
                uavs[i] = texture->CreateUAV({
                    .mipLevel = i
                });
            }

            ShaderParametersManager& shaderParametersManager = Engine::GetRenderer()->GetShaderParametersManager();
            for (Uint32 mipIndex = 1; mipIndex < texture->GetMipLevels(); ++mipIndex)
            {
                GPU_EVENT_SCOPE(mCommandList, Format<FrameString>(CUBE_T("GenerateMipmaps ({0}->{1})"), mipIndex - 1, mipIndex));

                width = std::max(1u, width >> 1);
                height = std::max(1u, height >> 1);

                SharedPtr<GenerateMipmapsShaderParameters> parameters = shaderParametersManager.CreateShaderParameters<GenerateMipmapsShaderParameters>();

                parameters->srcTexture.id = srvs[mipIndex - 1]->GetBindlessId();
                parameters->dstTexture.id = uavs[mipIndex]->GetBindlessId();
                mCommandList->UseResource(srvs[mipIndex - 1]);
                mCommandList->UseResource(uavs[mipIndex]);
                parameters->WriteAllParametersToBuffer();
                mCommandList->SetShaderVariableConstantBuffer(0, parameters->GetBuffer());

                Array<gapi::TransitionState, 2> transitions;
                transitions[0].resourceType = gapi::TransitionState::ResourceType::SRV;
                transitions[0].srv = srvs[mipIndex - 1];
                transitions[0].src = (mipIndex - 1 == 0) ? gapi::ResourceStateFlag::Common : gapi::ResourceStateFlag::UAV;
                transitions[0].dst = gapi::ResourceStateFlag::SRV_NonPixel;

                transitions[1].resourceType = gapi::TransitionState::ResourceType::UAV;
                transitions[1].uav = uavs[mipIndex];
                transitions[1].src = gapi::ResourceStateFlag::Common;
                transitions[1].dst = gapi::ResourceStateFlag::UAV;

                mCommandList->ResourceTransition(transitions);

                mCommandList->DispatchThreads(width, height, 1);
            }

            FrameVector<gapi::TransitionState> endTransitions;
            for (Uint32 i = 0; i < int(mipLevels) - 1; ++i)
            {
                endTransitions.push_back({
                    .resourceType = gapi::TransitionState::ResourceType::SRV,
                    .srv = srvs[i],
                    .src = gapi::ResourceStateFlag::SRV_NonPixel,
                    .dst = gapi::ResourceStateFlag::Common
                });
            }
            endTransitions.push_back({
                .resourceType = gapi::TransitionState::ResourceType::SRV,
                .srv = srvs[mipLevels - 1],
                .src = gapi::ResourceStateFlag::UAV,
                .dst = gapi::ResourceStateFlag::Common
            });
            mCommandList->ResourceTransition(endTransitions);
        }

        mCommandList->End();
        mCommandList->Submit();
    }
} // namespace cube
