#include "Texture.h"

#include "stb_image.h" // Loaded from tinyobjloader

#include "Allocator/FrameAllocator.h"
#include "Checker.h"
#include "Engine.h"
#include "GAPI_Texture.h"
#include "Renderer.h"

namespace cube
{
    TextureResource::TextureResource(const TextureResourceCreateInfo& createInfo)
    {
        const gapi::TextureInfo& info = createInfo.textureInfo;

        // TODO: Support otehr texture types.
        if (info.type != gapi::TextureType::Texture2D && info.type != gapi::TextureType::TextureCube)
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
                .arraySize = info.arraySize,
                .mipLevels = mipLevels,
            },
            .debugName = createInfo.debugName
        });

        // Set texture data
        const bool isTextureCube = (info.type == gapi::TextureType::TextureCube) ||
                                    (info.type == gapi::TextureType::TextureCubeArray);
        const Uint32 numSlices = isTextureCube ? info.arraySize * 6 : info.arraySize;

        CHECK(info.width * info.height * numSlices * createInfo.bytesPerElement == createInfo.data.GetSize());
        const Byte* pSrc = (const Byte*)createInfo.data.GetData();
        const Uint64 srcRowSize = info.width * createInfo.bytesPerElement;
        const Uint64 srcSubSize = info.width * info.height * createInfo.bytesPerElement;
            
        Byte* pDst = (Byte*)mGAPITexture->Map();
        // Only set miplevel 0.
        for (int i = 0; i < numSlices; ++i)
        {
            const int subresourceIndex = mGAPITexture->GetSubresourceIndex(i, 0);
            const gapi::SubresourceLayout& layout = mGAPITexture->GetSubresourceLayout(subresourceIndex);

            const Byte* pSrcSub = pSrc + srcSubSize * i;
            Byte* pDstSub = pDst + layout.offset;

            CHECK(srcRowSize <= layout.rowPitch);
            for (int y = 0; y < info.height; ++y)
            {
                memcpy(pDstSub + (y * layout.rowPitch), pSrcSub + (y * srcRowSize), srcRowSize);
            }
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

    TextureRawData TextureHelper::LoadFromFile(platform::FilePath path)
    {
        SharedPtr<platform::File> file = platform::FileSystem::OpenFile(path, platform::FileAccessModeFlag::Read);
        CHECK(file);
        const Uint64 fileSize = file->GetFileSize();
        FrameVector<stbi_uc> fileData(fileSize);
        file->Read(fileData.data(), fileSize);
        file = nullptr;

        int width, height, numChannels;
        stbi_info_from_memory(fileData.data(), fileSize, &width, &height, &numChannels);
        int desiredChannels = numChannels;
        if (numChannels == 3)
        {
            // Use RGBA16
            desiredChannels = 4;
        }
        stbi_us* data = stbi_load_16_from_memory(fileData.data(), fileSize, &width, &height, &numChannels, desiredChannels);

        Blob blobData(data, width * height * desiredChannels * sizeof(stbi_us));
        stbi_image_free(data);

        gapi::ElementFormat format = gapi::ElementFormat::Unknown;
        switch (desiredChannels)
        {
        case 1:
            format = gapi::ElementFormat::R16_UNorm;
            break;
        case 2:
            format = gapi::ElementFormat::RG16_UNorm;
            break;
        case 3: // Use RGBA16
        case 4:
            format = gapi::ElementFormat::RGBA16_UNorm;
            break;
        default:
            NO_ENTRY_FORMAT("Unsupported channel number.");
            format = gapi::ElementFormat::Unknown;
            break;
        }

        return {
            .format = format,
            .width = static_cast<Uint32>(width),
            .height = static_cast<Uint32>(height),
            .bytesPerElement = static_cast<Uint32>(sizeof(stbi_us) * desiredChannels),
            .data = std::move(blobData)
        };
    }
} // namespace cube
