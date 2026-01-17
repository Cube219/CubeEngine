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
            MetalShader(const MetalShaderCompileResult& result, StringView warningMessage, StringView errorMessage);
            virtual ~MetalShader();

            id<MTLFunction> GetMTLFunction() const { return mFunction; }

        private:
            id<MTLLibrary> mLibrary;
            id<MTLFunction> mFunction;
        };
    } // namespace gapi
} // namespace cube
