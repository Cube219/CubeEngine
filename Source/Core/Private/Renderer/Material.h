#pragma once

#include "CoreHeader.h"

#include "Renderer/RenderTypes.h"
#include "ShaderParameter.h"
#include "Vector.h"

namespace cube
{
    class TextureResource;

    namespace gapi
    {
        class Sampler;
        class Buffer;
        class Texture;
    } // namespace gapi

    class MaterialShaderParameters : public ShaderParameters
    {
        CUBE_BEGIN_SHADER_PARAMETERS(MaterialShaderParameters)
            CUBE_SHADER_PARAMETER(Vector4, baseColor)
            CUBE_SHADER_PARAMETER(int, useBaseColorTexture)
            CUBE_SHADER_PARAMETER(BindlessResource, baseColorTexture)
            CUBE_SHADER_PARAMETER(BindlessResource, sampler)
        CUBE_END_SHADER_PARAMETERS
    };

    class Material
    {
    public:
        Material(StringView debugName);
        ~Material();

        void SetBaseColor(Vector4 color);
        void SetBaseColorTexture(SharedPtr<TextureResource> texture);

        void SetSampler(int samplerIndex);

        SharedPtr<MaterialShaderParameters> GenerateShaderParameters() const;

    private:
        Vector4 mBaseColor;
        SharedPtr<TextureResource> mBaseColorTexture;
        int mSamplerIndex;
    };
} // namespace cube
