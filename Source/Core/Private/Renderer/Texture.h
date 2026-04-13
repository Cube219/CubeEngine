#pragma once

#include "CoreHeader.h"

#include "FileSystem.h"
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

    struct TextureRawData
    {
        gapi::ElementFormat format;
        Uint32 width;
        Uint32 height;
        Uint32 bytesPerElement;
        Blob data;
    };

    class TextureHelper
    {
    public:
        static TextureRawData LoadFromFile(platform::FilePath path);
    };
} // namespace cube
