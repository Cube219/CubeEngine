#pragma once

#include "CoreHeader.h"

#include "GAPI_Texture.h"

namespace cube
{
    struct TextureResourceCreateInfo
    {
        gapi::TextureType type;
        gapi::ElementFormat format;
        Uint32 width;
        Uint32 height;
        Uint32 depth = 1;
        Uint32 arraySize = 1;
        Uint32 mipLevels = 1;

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
        SharedPtr<gapi::TextureSRV> GetDefaultSRV() const { return mDefaultSRV; }

    private:
        SharedPtr<gapi::Texture> mGAPITexture;
        SharedPtr<gapi::TextureSRV> mDefaultSRV;

        String mDebugName;
    };
} // namespace cube
