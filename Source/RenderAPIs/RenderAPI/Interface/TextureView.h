#pragma once

#include "../RenderAPIHeader.h"

namespace cube
{
    namespace rapi
    {
        enum class TextureViewType
        {
            ShaderResourceView,
            RenderTargetView,
            DepthStencilView,
            UnorderedAccessView
        };

        struct TextureViewCreateInfo
        {
            TextureViewType type;

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
