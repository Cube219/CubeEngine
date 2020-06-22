#pragma once

#include "../VulkanAPIHeader.h"

#include "RenderAPIs/RenderAPI/Interface/Texture.h"

#include "../VulkanMemoryAllocator.h"

namespace cube
{
    namespace rapi
    {
        class TextureVk
        {
        public:
            TextureVk(VulkanDevice& device, ResourceUsage resUsage, VkImageViewType type, Uint64 size, const void* pData, TextureFormat format, Uint32 width, Uint32 height, Uint32 depth, Uint32 arraySize, TextureBindTypeFlags bindTypeFlags, Uint32 mipLevels, Uint32 samplesNum, const char* debugName);
            virtual ~TextureVk();

        private:
            VulkanDevice& mDevice;

            VkImage mImage;

            VulkanAllocation mAllocation;
        };

        class Texture2DVk : public Texture2D, public TextureVk
        {
        public:
            Texture2DVk(VulkanDevice& device, const Texture2DCreateInfo& info);
            virtual ~Texture2DVk() {}
        };

        class Texture2DArrayVk : public Texture2DArray, public TextureVk
        {
        public:
            Texture2DArrayVk(VulkanDevice& device, const Texture2DArrayCreateInfo& info);
            virtual ~Texture2DArrayVk() {}
        };

        class Texture3DVk : public Texture3D, public TextureVk
        {
        public:
            Texture3DVk(VulkanDevice& device, const Texture3DCreateInfo& info);
            virtual ~Texture3DVk() {}
        };

        class TextureCubeVk : public TextureCube, public TextureVk
        {
        public:
            TextureCubeVk(VulkanDevice& device, const TextureCubeCreateInfo& info);
            virtual ~TextureCubeVk() {}
        };
    } // namespace rapi
} // namespace cube
