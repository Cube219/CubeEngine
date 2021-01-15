#pragma once

#include "../VulkanAPIHeader.h"

#include <glslang/SPIRV/GlslangToSpv.h>

#include "Core/Allocator/FrameAllocator.h"
#include "RenderAPIs/RenderAPI/Interface/Resource.h"

namespace cube
{
    namespace rapi
    {
        class GLSLTool
        {
        public:
            GLSLTool() = delete;
            ~GLSLTool() = delete;

            static void Initialize();
            static void Shutdown();

            static void ToSPIRV(ShaderType type, const char* code, Vector<Uint32>& outSPIRV);

        private:
            static void InitResources();

            static TBuiltInResource mResources;
        };
    } // namespace rapi
} // namespace cube
