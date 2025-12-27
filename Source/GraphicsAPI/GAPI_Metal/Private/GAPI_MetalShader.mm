#include "GAPI_MetalShader.h"

#include "Allocator/FrameAllocator.h"
#include "Checker.h"
#include "Logger.h"
#include "MacOS/MacOSString.h"
#include "MetalDevice.h"
#include "ShaderParameterReflection.h"

namespace cube
{
    namespace gapi
    {
        MetalShader::MetalShader(const MetalShaderCompileResult& result, StringView warningMessage, StringView errorMessage) :
            mLibrary(result.library),
            mFunction(result.function)
        {
            mCreated = result.function != nil;
            mWarningMessage = warningMessage;
            mErrorMessage = errorMessage;

            const ShaderReflection& reflection = result.reflection;
            mThreadGroupSizeX = reflection.threadGroupSizeX;
            mThreadGroupSizeY = reflection.threadGroupSizeY;
            mThreadGroupSizeZ = reflection.threadGroupSizeZ;
        }

        MetalShader::~MetalShader()
        {
        }
    } // namespace gapi
} // namespace cube
