#include "GAPI_DX12Shader.h"

#include "DX12Utility.h"
#include "SlangHelper.h"

namespace cube
{
    namespace gapi
    {
        DX12Shader::DX12Shader(DX12ShaderCompilerResult&& result, StringView warningMessage, StringView errorMessage, Vector<String>&& dependencyFilePaths)
            : mShader(std::move(result.shader))
        {
            mCreated = mShader.GetSize() > 0;
            mWarningMessage = warningMessage;
            mErrorMessage = errorMessage;
            mDependencyFilePaths = std::move(dependencyFilePaths);

            mThreadGroupSizeX = result.reflection.threadGroupSizeX;
            mThreadGroupSizeY = result.reflection.threadGroupSizeY;
            mThreadGroupSizeZ = result.reflection.threadGroupSizeZ;
        }

        DX12Shader::~DX12Shader()
        {
            mShader.Release();
        }
    } // namespace gapi
} // namespace cube
