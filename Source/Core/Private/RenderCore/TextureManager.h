#pragma once

#include "CoreHeader.h"

#include "GAPI_Buffer.h"
#include "RenderCore/RenderGraphTypes.h"
#include "RenderCore/RenderTypes.h"
#include "RenderCore/ShaderParameter.h"
#include "Pipeline.h"

namespace cube
{
    class GAPI;
    class Renderer;
    class TextureResource;

    namespace gapi
    {
        class CommandList;
        class Fence;
        class Texture;
    } // namespace gapi

    class GenerateMipmapsShaderParameterList : public ShaderParameterList
    {
        CUBE_BEGIN_SHADER_PARAMETER_LIST(GenerateMipmapsShaderParameterList)
            CUBE_SHADER_PARAMETER(RGTextureSRVHandle, srcTexture)
            CUBE_SHADER_PARAMETER(RGTextureUAVHandle, dstTexture)
        CUBE_END_SHADER_PARAMETER_LIST
    };

    class TextureManager
    {
    public:
        TextureManager(Renderer& renderer);
        ~TextureManager() = default;

        void Initialize(GAPI* gapi, Uint32 numGPUSync);
        void Shutdown();

        void GenerateMipmaps(SharedPtr<gapi::Texture> texture, gapi::CommandList& commandList);

        SharedPtr<gapi::CommandList> GetTextureInitCommandList() const { return mTextureInitCommandList; }
        SharedPtr<gapi::Fence> GetTextureInitFence() const { return mTextureInitFence; }
        Uint64 GetAndMoveTextureInitFenceValue()
        {
            mTextureInitFenceLastValue++;
            return mTextureInitFenceLastValue;
        }

    private:
        GAPI* mGAPI;
        Renderer& mRenderer;

        SharedPtr<Shader> mGenerateMipmapsShader;
        ComputePipelineInfo mGenerateMipmapsPipelineInfo;

        SharedPtr<gapi::CommandList> mTextureInitCommandList;
        SharedPtr<gapi::Fence> mTextureInitFence;
        Uint64 mTextureInitFenceLastValue = 0;
    };
} // namespace cube
