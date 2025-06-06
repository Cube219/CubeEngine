#pragma once

#include "MetalHeader.h"

#include "GAPI_Texture.h"

namespace cube
{
    namespace gapi
    {
        class MetalTexture : public Texture
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

        private:
            void* mTempBuffer;
        };
    } // namespace gapi
} // namespace cube
