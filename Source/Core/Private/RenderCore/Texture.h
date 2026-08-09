#pragma once

#include "CoreHeader.h"

#include "FileSystem.h"
#include "GAPI_Texture.h"

namespace cube
{
    namespace gapi
    {
        class CommandList;
    } // namespace gapi

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

        void WaitUntilInitialized();
        void WaitUntilInitialized(gapi::CommandList& commandList);

    private:
        void CheckInitialized();

        SharedPtr<gapi::Texture> mGAPITexture;

        Uint64 mInitFenceValue;
        bool mIsInitialized = false;

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
