#pragma once

#include "DX12Header.h"

#include "GAPI_Shader.h"

#include "DX12APIObject.h"
#include "DX12ShaderCompiler.h"

namespace cube
{
    namespace gapi
    {
        class DX12Shader : public Shader, public DX12APIObject
        {
        public:
            DX12Shader(DX12ShaderCompilerResult&& result, StringView warningMessage, StringView errorMessage);
            virtual ~DX12Shader();

            BlobView GetShader() const { return mShader; }

        private:
            Blob mShader;
        };
    } // namespace gapi
} // namespace cube
