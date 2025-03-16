#pragma once

#include "DX12Header.h"

#include "GAPI_Shader.h"

namespace cube
{
    namespace gapi
    {
        class DX12Shader : public Shader
        {
        public:
            DX12Shader(const ShaderCreateInfo& info);
            virtual ~DX12Shader();

            ID3DBlob* GetShader() const { return mShader; }

        private:
            void LoadFromHLSL(const ShaderCreateInfo& info);
            void LoadFromDXIL(const ShaderCreateInfo& info);

            ID3DBlob* mShader;
        };
    } // namespace gapi
} // namespace cube
