#pragma once

#include "CoreHeader.h"

#include "GAPI_Buffer.h"
#include "Renderer/RenderGraphTypes.h"
#include "Renderer/RenderTypes.h"
#include "Renderer/ShaderParameter.h"

namespace cube
{
    class ComputePipeline;
    class GAPI;
    class Renderer;
    class Shader;
    class TextureResource;

    namespace gapi
    {
        class CommandList;
        class Texture;
    } // namespace gapi

    class GenerateMipmapsShaderParameters : public ShaderParameters
    {
        CUBE_BEGIN_SHADER_PARAMETERS(GenerateMipmapsShaderParameters)
            CUBE_SHADER_PARAMETER(RGTextureSRVHandle, srcTexture)
            CUBE_SHADER_PARAMETER(RGTextureUAVHandle, dstTexture)
        CUBE_END_SHADER_PARAMETERS
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
        SharedPtr<ComputePipeline> mGenerateMipmapsPipeline;

        SharedPtr<gapi::CommandList> mCommandList;
    };
} // namespace cube
