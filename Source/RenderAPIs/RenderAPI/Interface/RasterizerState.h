#pragma once

#include "../RenderAPIHeader.h"

namespace cube
{
    namespace rapi
    {
        enum class FillMode
        {
            Point,
            Line,
            Solid
        };

        enum class CullMode
        {
            None,
            Front,
            Back
        };

        enum class FrontFace
        {
            Clockwise,
            Counterclockwise
        };

        struct RasterizerState
        {
            FillMode fillMode = FillMode::Solid;
            CullMode cullMode = CullMode::Back;
            FrontFace frontFace = FrontFace::Clockwise;
        };
    } // namespace rapi
} // namespace cube
