#pragma once

#include "MetalHeader.h"

#include "GAPI_Shader.h"

namespace cube
{
    namespace gapi
    {
        class MetalShader : public Shader
        {
        public:
            MetalShader(const ShaderCreateInfo& info) {}
            virtual ~MetalShader() {}
        };
    } // namespace gapi
} // namespace cube
