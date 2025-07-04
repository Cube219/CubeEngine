#include "Texture.h"

#include "Checker.h"
#include "Engine.h"
#include "GAPI_Texture.h"
#include "Renderer.h"

namespace cube
{
    Texture::Texture(const TextureCreateInfo& createInfo)
    {
        mGAPITexture = Engine::GetRenderer()->GetGAPI().CreateTexture({
            .usage = gapi::ResourceUsage::GPUOnly,
            .format = createInfo.format,
            .type = createInfo.type,
            .width = createInfo.width,
            .height = createInfo.height,
            .debugName = createInfo.debugName
        });

        // Set texture data
        CHECK(createInfo.width * createInfo.height * createInfo.bytesPerElement == createInfo.data.GetSize());
        const Byte* pSrc = (const Byte*)createInfo.data.GetData();
        Uint64 srcRowSize = createInfo.width * createInfo.bytesPerElement;
            
        Byte* pDst = (Byte*)mGAPITexture->Map();
        Uint64 dstRowPitch = mGAPITexture->GetRowPitch();

        CHECK(srcRowSize <= dstRowPitch);

        for (int y = 0; y < createInfo.height; ++y)
        {
            memcpy(pDst + (y * dstRowPitch), pSrc + (y * srcRowSize), srcRowSize);
        }
        mGAPITexture->Unmap();
    }

    Texture::~Texture()
    {
        mGAPITexture = nullptr;
    }
} // namespace cube
