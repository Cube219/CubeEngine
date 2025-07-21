#pragma once

#include "CoreHeader.h"

#include "GAPI_Buffer.h"
#include "Renderer/RenderTypes.h"
#include "ShaderParameter.h"

namespace cube
{
    class GAPI;
    class TextureResource;

    namespace gapi
    {
        class CommandList;
        class ComputePipeline;
        class Shader;
        class ShaderVariablesLayout;
        class Texture;
    } // namespace gapi

    class TextureManager
    {
    public:
        TextureManager() = default;
        ~TextureManager() = default;

        void Initialize(GAPI* gapi, Uint32 numGPUSync);
        void Shutdown();

        void GenerateMipmaps(SharedPtr<gapi::Texture> texture);

    private:
        GAPI* mGAPI;

        Uint32 mCurrentIndex;

        SharedPtr<gapi::ShaderVariablesLayout> mGenerateMipmapsShaderVariablesLayout;
        SharedPtr<gapi::Shader> mGenerateMipmapsShader;
        SharedPtr<gapi::ComputePipeline> mGenerateMipmapsPipeline;

        class GenerateMipmapsShaderParameters : public ShaderParameters
        {
            CUBE_BEGIN_SHADER_PARAMETERS(GenerateMipmapsShaderParameters)
                CUBE_SHADER_PARAMETER(BindlessResource, srcTexture)
                CUBE_SHADER_PARAMETER(BindlessResource, dstTexture)
            CUBE_END_SHADER_PARAMETERS
        };

        SharedPtr<gapi::CommandList> mCommandList;
    };
} // namespace cube
