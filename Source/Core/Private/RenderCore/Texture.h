#pragma once

#include "CoreHeader.h"

#include "FileSystem.h"
#include "GAPI_Texture.h"
#include "Resource.h"

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

    class TextureResource : public Resource
    {
    public:
        TextureResource(const TextureResourceCreateInfo& createInfo);
        ~TextureResource();

        SharedPtr<gapi::Texture> GetGAPITexture() const { return mGAPITexture; }

        static SharedPtr<TextureResource> Create(const TextureResourceCreateInfo& createInfo);

    private:
        SharedPtr<gapi::Texture> mGAPITexture;
        Uint64 mUploadFinishFenceValue = 0;

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
        enum class LoadElementType
        {
            U8,
            U16,
            Float
        };

        static TextureRawData LoadFromFile(platform::FilePath path, LoadElementType loadElementType = LoadElementType::U8);
    };
} // namespace cube
