#pragma once

#include "../RenderAPIHeader.h"

namespace cube
{
    namespace rapi
    {
        struct TextureViewCreateInfo
        {
            TextureViewType type;
            TextureViewDimension dimension;

            TextureFormat format = TextureFormat::Unknown;
            Uint32 mipmapStartIndex = 0;
            Uint32 numMipmaps = 0;
            Uint32 arrayStartIndex = 0;
            Uint32 numArrays;

            const char* debugName = "";
        };

        class TextureView
        {
        public:
            TextureView() = default;
            virtual ~TextureView() = default;
        };
    } // namespace rapi
} // namespace cube
