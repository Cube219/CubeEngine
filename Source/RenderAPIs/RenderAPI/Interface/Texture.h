#pragma once

#include "../RenderAPIHeader.h"

#include "Resource.h"
#include "Core/Allocator/FrameAllocator.h"

namespace cube
{
    namespace rapi
    {
        struct TextureData
        {
            const void* pData;
            Uint32 size;
        };

        struct TextureCreateInfo : public ResourceCreateInfo
        {
            TextureDimension dimension;
            Uint32 width;
            Uint32 height;
            Uint32 depth;
            Uint32 arraySize;
            FrameVector<TextureData> data;

            TextureFormat format;
            TextureBindTypeFlags bindTypeFlags;
            bool generateMipmaps; // If it is true, only the first texture data will be used
            Uint32 samplesNum;
        };

        class Texture : public Resource
        {
        public:
            Texture(ResourceUsage usage, const char* debugName) :
                Resource(usage, debugName)
            {}
            virtual ~Texture() = default;

            TextureFormat GetFormat() const { return mFormat; }
            Uint32 GetMipLevels() const { return mMipLevels; }

            virtual void Map(ResourceMapType type, Uint32 mipmapIndex, void*& pMappedResource) = 0;
            virtual void Unmap(Uint32 mipmapIndex) = 0;
            virtual void GenerateMipmaps() = 0;
            virtual SPtr<Fence> MapAsync(ResourceMapType type, Uint32 mipmapIndex, void*& pMappedResource) = 0;
            virtual SPtr<Fence> UnmapAsync(Uint32 mipmapIndex) = 0;
            virtual SPtr<Fence> GenerateMipmapsAsync() = 0;

            virtual SPtr<TextureView> CreateView(const TextureViewCreateInfo& info) = 0;
            virtual SPtr<TextureView> CreateDefaultView() = 0;

        protected:
            TextureFormat mFormat;
            TextureBindTypeFlags mBindTypeFlags;
            Uint32 mMipLevels;
            Uint32 mSamplesNum;
        };
    } // namespace rapi
} // namespace cube
