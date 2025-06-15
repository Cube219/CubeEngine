#include "Material.h"

#include "Engine.h"
#include "GAPI_Buffer.h"
#include "GAPI_Texture.h"
#include "Renderer.h"

namespace cube
{
    Material::Material() :
        mBaseColorTexture(nullptr)
    {
        mBuffer = Engine::GetRenderer()->GetGAPI().CreateBuffer({
            .type = gapi::BufferType::Constant,
            .usage = gapi::ResourceUsage::CPUtoGPU,
            .size = sizeof(BufferData),
            .debugName = "Material buffer data"
        });
        mBufferDataPointer = (Byte*)mBuffer->Map();

        mBufferData.sampler.index = Engine::GetRenderer()->GetSamplerManager().GetDefaultLinearSamplerIndex();

        UpdateBufferData();
    }

    Material::~Material()
    {
        mBuffer = nullptr;
    }

    void Material::SetBaseColor(Vector4 color)
    {
        mBufferData.baseColor = color;

        UpdateBufferData();
    }

    void Material::SetBaseColorTexture(SharedPtr<gapi::Texture> texture)
    {
        if (texture != nullptr)
        {
            mBaseColorTexture = texture;
            mBufferData.useBaseColorTexture = true;
            mBufferData.baseColorTexture.index = texture->GetBindlessIndex();
        }
        else
        {
            mBaseColorTexture = nullptr;
            mBufferData.useBaseColorTexture = false;
            mBufferData.baseColorTexture = InvalidBindlessResource;
        }

        UpdateBufferData();
    }

    void Material::SetSampler(int samplerIndex)
    {
        mBufferData.sampler.index = samplerIndex;

        UpdateBufferData();
    }

    void Material::UpdateBufferData()
    {
        memcpy(mBufferDataPointer, &mBufferData, sizeof(BufferData));
    }
} // namespace cube
