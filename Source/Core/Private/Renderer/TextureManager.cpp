#include "TextureManager.h"

#include "Checker.h"
#include "Engine.h"
#include "FileSystem.h"
#include "GAPI.h"
#include "GAPI_CommandList.h"
#include "GAPI_Pipeline.h"
#include "GAPI_Shader.h"
#include "GAPI_ShaderVariable.h"
#include "Texture.h"
#include "Allocator/FrameAllocator.h"

namespace cube
{
    void TextureManager::Initialize(GAPI* gapi, Uint32 numGPUSync)
    {
        mGAPI = gapi;
        mGenerateMipmapsBufferList.resize(numGPUSync);

        {
            mGenerateMipmapsShaderVariablesLayout = mGAPI->CreateShaderVariablesLayout({
                .numShaderVariablesConstantBuffer = 1,
                .shaderVariablesConstantBuffer = nullptr,
                .debugName = CUBE_T("GenerateMipmapsShaderVariablesLayout")
            });
        
            FrameString shaderFileName = CUBE_T("GenerateMipmaps.slang");
            FrameString shaderFilePath = Format<FrameString>(CUBE_T("{0}/Resources/Shaders/{1}"), Engine::GetRootDirectoryPath(), shaderFileName);
            SharedPtr<platform::File> shaderFile = platform::FileSystem::OpenFile(shaderFilePath, platform::FileAccessModeFlag::Read);
            CHECK(shaderFile);
            Uint64 shaderFileSize = shaderFile->GetFileSize();
        
            Blob shaderCode(shaderFileSize);
            Uint64 readSize = shaderFile->Read(shaderCode.GetData(), shaderFileSize);
            CHECK(readSize <= shaderFileSize);
        
            mGenerateMipmapsShader = mGAPI->CreateShader({
                .type = gapi::ShaderType::Compute,
                .language = gapi::ShaderLanguage::Slang,
                .fileName = shaderFileName,
                .path = shaderFilePath,
                .code = shaderCode,
                .entryPoint = "CSMain",
                .withDebugSymbol = true, // TODO: Add option in render ui after implement shader recompilation,
                .debugName = CUBE_T("GenerateMipmapsShaderCS")
            });
            CHECK(mGenerateMipmapsShader);
        
            mGenerateMipmapsPipeline = mGAPI->CreateComputePipeline({
                .shader = mGenerateMipmapsShader,
                .shaderVariablesLayout = mGenerateMipmapsShaderVariablesLayout,
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

        mGenerateMipmapsBufferList.clear();

        mGenerateMipmapsPipeline = nullptr;
        mGenerateMipmapsShader = nullptr;
        mGenerateMipmapsShaderVariablesLayout = nullptr;
    }
    void TextureManager::MoveNextFrame()
    {
        mCurrentIndex = (mCurrentIndex + 1) % mGenerateMipmapsBufferList.size();

        mGenerateMipmapsBufferList[mCurrentIndex].clear();
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
        
        mCommandList->SetShaderVariablesLayout(mGenerateMipmapsShaderVariablesLayout);
        mCommandList->SetComputePipeline(mGenerateMipmapsPipeline);

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
        for (Uint32 mipIndex = 1; mipIndex < texture->GetMipLevels(); ++mipIndex)
        {
            width = std::max(1u, width >> 1);
            height = std::max(1u, height >> 1);

            GenerateMipmapBuffer newBuffer;
            newBuffer.buffer = mGAPI->CreateBuffer({
                .type = gapi::BufferType::Constant,
                .usage = gapi::ResourceUsage::CPUtoGPU,
                .size = sizeof(GenerateMipmapsBufferData),
                .debugName = CUBE_T("GenerateMipmapsBuffer")
            });
            newBuffer.bufferPointer = (Byte*)newBuffer.buffer->Map();
            mGenerateMipmapsBufferList[mCurrentIndex].push_back(std::move(newBuffer));

            GenerateMipmapBuffer& buffer = mGenerateMipmapsBufferList[mCurrentIndex].back();
            buffer.data.srcTexture.index = srvs[mipIndex - 1]->GetBindlessIndex();
            buffer.data.dstTexture.index = uavs[mipIndex]->GetBindlessIndex();
            memcpy(buffer.bufferPointer, &buffer.data, sizeof(GenerateMipmapsBufferData));
            mCommandList->SetShaderVariableConstantBuffer(0, buffer.buffer);

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

            mCommandList->Dispatch((width + 7) / 8, (height + 7) / 8, 1);
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

        mCommandList->End();
        mCommandList->Submit();
    }
} // namespace cube
