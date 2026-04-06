#pragma once

#include "CoreHeader.h"

#include "GAPI_Texture.h"

namespace cube
{
    struct TextureResourceCreateInfo
    {
        gapi::TextureInfo textureInfo;

        BlobView data;
        Uint32 bytesPerElement;
        bool generateMipMaps = false;

        StringView debugName;
    };

    class TextureResource
    {
    public:
        TextureResource(const TextureResourceCreateInfo& createInfo);
        ~TextureResource();

        SharedPtr<gapi::Texture> GetGAPITexture() const { return mGAPITexture; }

    private:
        SharedPtr<gapi::Texture> mGAPITexture;

        String mDebugName;
    };
} // namespace cube
