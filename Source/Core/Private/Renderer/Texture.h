#pragma once

#include "CoreHeader.h"

#include "GAPI_Texture.h"

namespace cube
{
    struct TextureCreateInfo
    {
        gapi::TextureType type;
        gapi::ElementFormat format;
        Uint32 width;
        Uint32 height;
        Uint32 depth = 1;
        Uint32 arraySize = 1;
        Uint32 mipLevels = 0;

        BlobView data;
        Uint32 bytesPerElement;
        bool generateMipMaps = false;

        StringView debugName;
    };

    class Texture
    {
    public:
        Texture(const TextureCreateInfo& createInfo);
        ~Texture();

        SharedPtr<gapi::Texture> GetGAPITexture() const { return mGAPITexture; }

    private:
        SharedPtr<gapi::Texture> mGAPITexture;

        String mDebugName;
    };
} // namespace cube
