#include "Material.h"

#include "Allocator/FrameAllocator.h"
#include "Engine.h"
#include "GAPI_Buffer.h"
#include "GAPI_Texture.h"
#include "Renderer.h"
#include "Texture.h"

namespace cube
{
    Material::Material(StringView debugName) :
        mBaseColorTexture(nullptr)
    {
        FrameString bufferDebugName = Format<FrameString>(CUBE_T("[{0}] MaterialBuffer"), debugName);

        mSamplerIndex = Engine::GetRenderer()->GetSamplerManager().GetDefaultLinearSamplerIndex();
    }

    Material::~Material()
    {
    }

    void Material::SetBaseColor(Vector4 color)
    {
        mBaseColor = color;
    }

    void Material::SetBaseColorTexture(SharedPtr<TextureResource> texture)
    {
        mBaseColorTexture = texture;
    }

    void Material::SetSampler(int samplerIndex)
    {
        mSamplerIndex = samplerIndex;
    }

    SharedPtr<MaterialShaderParameters> Material::GenerateShaderParameters() const
    {
        ShaderParametersManager& shaderParametersManager = Engine::GetRenderer()->GetShaderParametersManager();

        SharedPtr<MaterialShaderParameters> parameters = shaderParametersManager.CreateShaderParameters<MaterialShaderParameters>();
        parameters->baseColor = mBaseColor;
        parameters->useBaseColorTexture = 0;
        if (mBaseColorTexture)
        {
            parameters->useBaseColorTexture = 1;
            parameters->baseColorTexture.index = mBaseColorTexture->GetDefaultSRV()->GetBindlessIndex();
        }
        parameters->sampler.index = mSamplerIndex;
        parameters->WriteAllParametersToBuffer();

        return parameters;
    }
} // namespace cube
