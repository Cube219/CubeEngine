#include "GAPI_DX12Shader.h"

#include "DX12Utility.h"
#include "SlangHelper.h"

namespace cube
{
    namespace gapi
    {
        DX12Shader::DX12Shader(Blob&& shader) :
            mShader(std::move(shader))
        {
        }

        DX12Shader::~DX12Shader()
        {
            mShader.Release();
        }
    } // namespace gapi
} // namespace cube
