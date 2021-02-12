#pragma once

#include "../RenderAPIHeader.h"

namespace cube
{
    namespace rapi
    {
        enum class CompareFunction
        {
            Less,
            LessEqual,
            Greater,
            GreaterEqual,
            Equal,
            NotEqual,
            Never,
            Always
        };

        enum class StencilOperator
        {
            Keep,
            Zero,
            Replace,
            Invert,
            IncrementAndClamp,
            DecrementAndClamp,
            IncrementAndWrap,
            DecrementAndWrap
        };

        struct DepthStencilState
        {
            bool enableDepthWrite;
            CompareFunction depthFunction;

            bool enableFrontFaceStencil;
            CompareFunction frontFaceStencilFunction;
            StencilOperator frontFaceStencilFailOp;
            StencilOperator frontFaceStencilPassOp;
            StencilOperator frontFaceStencilDepthFailOp;

            bool enableBackFaceStencil;
            CompareFunction backFaceStencilFunction;
            StencilOperator backFaceStencilFailOp;
            StencilOperator backFaceStencilPassOp;
            StencilOperator backFaceStencilDepthFailOp;

            Uint8 stencilReadMask;
            Uint8 stencilWriteMask;
        };
    } // namespace rapi
} // namespace cube
