#pragma once

#include "MetalHeader.h"

#include "GAPI_Texture.h"

namespace cube
{
    namespace gapi
    {
        class MetalTexture : public Texture, public std::enable_shared_from_this<MetalTexture>
        {
        public:
            MetalTexture(const TextureCreateInfo& info) :
                Texture(info)
            {
                mRowPitch = info.width * 4;
                mTempBuffer = malloc(info.width * info.height * info.depth * 4);
            }
            virtual ~MetalTexture()
            {
                free(mTempBuffer);
            }

            virtual void* Map() override { return mTempBuffer; }
            virtual void Unmap() override {}

            virtual SharedPtr<TextureSRV> CreateSRV(const TextureSRVCreateInfo& createInfo) override
            {
                return std::make_shared<TextureSRV>(createInfo, shared_from_this());
            }
            virtual SharedPtr<TextureUAV> CreateUAV(const TextureUAVCreateInfo& createInfo) override
            {
                return std::make_shared<TextureUAV>(createInfo, shared_from_this());
            }

        private:
            void* mTempBuffer;
        };
    } // namespace gapi
} // namespace cube
