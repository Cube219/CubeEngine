#pragma once

#include "../RenderAPIHeader.h"

namespace cube
{
    namespace rapi
    {
        enum class InputRate
        {
            PerVertex,
            PerInstance
        };

        struct InputLayout
        {
            struct Element
            {
                Uint32 offset = 0;
                TextureFormat format = TextureFormat::RGBA_32_Float;
            };

            Uint32 stride = 0;
            const Element* elements = nullptr;
            Uint32 numElements = 0;

            InputRate inputRate = InputRate::PerVertex;
        };
    } // namespace rapi
} // namespace cube
