#include "Texture.h"

#include "Checker.h"
#include "Engine.h"
#include "GAPI_Texture.h"
#include "Renderer.h"

namespace cube
{
    TextureResource::TextureResource(const TextureResourceCreateInfo& createInfo)
    {
        // TODO: Currently only 2d. Consider other dimensions?
        if (createInfo.type != gapi::TextureType::Texture2D)
        {
            NOT_IMPLEMENTED();
        }

        Uint32 mipLevels = createInfo.mipLevels;
        if (createInfo.generateMipMaps)
        {
            Uint32 width = createInfo.width;
            Uint32 height = createInfo.height;
            mipLevels = 0;
            while (width > 0 && height > 0)
            {
                mipLevels++;
                width >>= 1;
                height >>= 1;
            }
        }

        mGAPITexture = Engine::GetRenderer()->GetGAPI().CreateTexture({
            .usage = gapi::ResourceUsage::GPUOnly,
            .format = createInfo.format,
            .type = createInfo.type,
            .flags = createInfo.generateMipMaps ? gapi::TextureFlag::UAV : gapi::TextureFlag::None,
            .width = createInfo.width,
            .height = createInfo.height,
            .mipLevels = mipLevels,
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

        if (createInfo.generateMipMaps)
        {
            Engine::GetRenderer()->GetTextureManager().GenerateMipmaps(mGAPITexture);
        }

        mDefaultSRV = mGAPITexture->CreateSRV({});
    }

    TextureResource::~TextureResource()
    {
        mDefaultSRV = nullptr;
        mGAPITexture = nullptr;
    }
} // namespace cube
