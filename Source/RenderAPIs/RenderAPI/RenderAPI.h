#pragma once

#include "RenderAPIHeader.h"

namespace cube
{
    namespace rapi
    {
        struct RenderAPICreateInfo
        {
            bool enableDebugLayer = false;

            Uint32 numGraphicsCommandPools = 4;
            Uint32 numComputeCommandPools = 1;
            Uint32 numTransferCommandPools = 4;
        };

        class RenderAPI
        {
        public:
            RenderAPI() {}
            virtual ~RenderAPI() {}

            virtual void Initialize(const RenderAPICreateInfo& info) = 0;
            virtual void Shutdown() = 0;

            virtual SPtr<Texture> CreateTexture(const TextureCreateInfo& info) = 0;
        };
    } // namespace rapi
} // namespace cube
