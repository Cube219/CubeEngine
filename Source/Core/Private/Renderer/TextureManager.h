#pragma once

#include "CoreHeader.h"

#include "GAPI_Buffer.h"
#include "Renderer/RenderGraphTypes.h"
#include "Renderer/RenderTypes.h"
#include "Renderer/ShaderParameter.h"
#include "Shader.h"

namespace cube
{
    class GAPI;
    class Renderer;
    class TextureResource;

    namespace gapi
    {
        class CommandList;
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

        void GenerateMipmaps(SharedPtr<gapi::Texture> texture);

    private:
        GAPI* mGAPI;
        Renderer& mRenderer;

        SharedPtr<Shader> mGenerateMipmapsShader;
        ComputePipelineInfo mGenerateMipmapsPipelineInfo;

        SharedPtr<gapi::CommandList> mCommandList;
    };
} // namespace cube
