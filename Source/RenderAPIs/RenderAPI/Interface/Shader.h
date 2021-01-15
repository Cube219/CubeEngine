#pragma once

#include "../RenderAPIHeader.h"

namespace cube
{
    namespace rapi
    {
        struct ShaderCreateInfo
        {
            ShaderType type;
            ShaderLanguage language;

            const char* code;

            const char* debugName = nullptr;
        };

        class Shader
        {
        public:
            Shader(const char* debugName) {}
            virtual ~Shader() {}

        protected:
            const char* mDebugName = nullptr;
        };
    } // namespace rapi
} // namespace cube
