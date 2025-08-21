#include "GAPI_DX12Shader.h"

#include "DX12Utility.h"
#include "SlangHelper.h"

namespace cube
{
    namespace gapi
    {
        DX12Shader::DX12Shader(Blob&& shader, StringView warningMessage, StringView errorMessage) :
            mShader(std::move(shader))
        {
            mCreated = mShader.GetSize() > 0;
            mWarningMessage = warningMessage;
            mErrorMessage = errorMessage;
        }

        DX12Shader::~DX12Shader()
        {
            mShader.Release();
        }
    } // namespace gapi
} // namespace cube
