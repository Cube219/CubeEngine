#include "Texture.h"

#include "Checker.h"
#include "Engine.h"
#include "GAPI_Texture.h"
#include "Renderer.h"

namespace cube
{
    TextureResource::TextureResource(const TextureResourceCreateInfo& createInfo)
    {
        const gapi::TextureInfo& info = createInfo.textureInfo;

        // TODO: Currently only 2d. Consider other dimensions?
        if (info.type != gapi::TextureType::Texture2D)
        {
            NOT_IMPLEMENTED();
        }

        Uint32 mipLevels = info.mipLevels;
        if (createInfo.generateMipMaps)
        {
            Uint32 width = info.width;
            Uint32 height = info.height;
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
            .textureInfo = {
                .format = info.format,
                .type = info.type,
                .flags = createInfo.generateMipMaps ? gapi::TextureFlag::UAV : gapi::TextureFlag::None,
                .width = info.width,
                .height = info.height,
                .mipLevels = mipLevels,
            },
            .debugName = createInfo.debugName
        });

        // Set texture data
        CHECK(info.width * info.height * createInfo.bytesPerElement == createInfo.data.GetSize());
        const Byte* pSrc = (const Byte*)createInfo.data.GetData();
        Uint64 srcRowSize = info.width * createInfo.bytesPerElement;
            
        Byte* pDst = (Byte*)mGAPITexture->Map();
        Uint64 dstRowPitch = mGAPITexture->GetRowPitch();

        CHECK(srcRowSize <= dstRowPitch);

        for (int y = 0; y < info.height; ++y)
        {
            memcpy(pDst + (y * dstRowPitch), pSrc + (y * srcRowSize), srcRowSize);
        }
        mGAPITexture->Unmap();

        if (createInfo.generateMipMaps)
        {
            Engine::GetRenderer()->GetTextureManager().GenerateMipmaps(mGAPITexture);
        }
    }

    TextureResource::~TextureResource()
    {
        mGAPITexture = nullptr;
    }
} // namespace cube
