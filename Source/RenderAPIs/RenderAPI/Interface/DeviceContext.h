#pragma once

#include "../RenderAPIHeader.h"

namespace cube
{
    namespace rapi
    {
        struct Viewport
        {
            float x;
            float y;
            float width;
            float height;
            float minDepth;
            float maxDepth;
        };

        struct Rect2D
        {
            Int32 x;
            Int32 y;
            Uint32 width;
            Uint32 height;
        };

        class DeviceContext
        {
        public:
            DeviceContext() {}
            virtual ~DeviceContext() {}

            virtual void SetViewports(Uint32 numViewports, const Viewport* pViewports) = 0;

            virtual void SetScissors(Uint32 numScissors, const Rect2D* pScissors) = 0;

            // SetPipelineState
            
            // SetRenderTargets

            // SetShaderTexture

            // BindVertexBuffer
            // BindIndexBuffer
            
            // Draw
            // Dispatch

            // Submit
        };
    } // namespace rapi
} // namespace cube
