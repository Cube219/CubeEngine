#include "GAPI_DX12Shader.h"

#include "Checker.h"
#include "DX12Utility.h"
#include "SlangHelper.h"

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
