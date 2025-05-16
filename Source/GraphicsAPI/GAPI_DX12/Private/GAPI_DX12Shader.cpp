#include "GAPI_DX12Shader.h"

#include "Checker.h"
#include "DX12Utility.h"
#include "SlangHelper.h"

#ifndef CUBE_DX12_SLANG_COMPILE_TO_HLSL
#define CUBE_DX12_SLANG_COMPILE_TO_HLSL 0
#endif

namespace cube
{
    namespace gapi
    {
        DX12Shader::DX12Shader(ID3DBlob* shader) :
            mShader(shader)
        {
        }

        DX12Shader::~DX12Shader()
        {
            if (mShader)
            {
                mShader->Release();
            }
        }
    } // namespace gapi
} // namespace cube
