#pragma once

#include "RenderAPIHeader.h"

namespace cube
{
    namespace rapi
    {
        class RenderAPI
        {
        public:
            RenderAPI() {}
            virtual ~RenderAPI() {}

            virtual void Initialize(bool enableDebugLayer = false) = 0;
            virtual void Shutdown() = 0;

            virtual SPtr<Texture2D> CreateTexture2D(const Texture2DCreateInfo& info) = 0;
            virtual SPtr<Texture2DArray> CreateTexture2DArray(const Texture2DArrayCreateInfo& info) = 0;
            virtual SPtr<Texture3D> CreateTexture3D(const Texture3DCreateInfo& info) = 0;
            virtual SPtr<TextureCube> CreateTextureCube(const TextureCubeCreateInfo& info) = 0;
        };
    } // namespace rapi
} // namespace cube
