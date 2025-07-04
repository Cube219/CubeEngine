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
        mBuffer = Engine::GetRenderer()->GetGAPI().CreateBuffer({
            .type = gapi::BufferType::Constant,
            .usage = gapi::ResourceUsage::CPUtoGPU,
            .size = sizeof(BufferData),
            .debugName = bufferDebugName
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

    void Material::SetBaseColorTexture(SharedPtr<TextureResource> texture)
    {
        if (texture != nullptr)
        {
            mBaseColorTexture = texture;
            mBufferData.useBaseColorTexture = true;
            mBufferData.baseColorTexture.index = texture->GetDefaultSRV()->GetBindlessIndex();
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
