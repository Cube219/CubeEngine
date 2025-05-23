#pragma once

#include "MetalHeader.h"

#include "GAPI_Shader.h"
#include "MetalShaderCompiler.h"

namespace cube
{
    class MetalDevice;

    namespace gapi
    {
        class MetalShader : public Shader
        {
        public:
            MetalShader(MetalShaderCompileResult result);
            virtual ~MetalShader();

        private:
            id<MTLLibrary> mLibrary;
            id<MTLFunction> mFunction;
        };
    } // namespace gapi
} // namespace cube
