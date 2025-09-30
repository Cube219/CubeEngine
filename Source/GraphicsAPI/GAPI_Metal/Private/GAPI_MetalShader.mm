#include "GAPI_MetalShader.h"

#include "Allocator/FrameAllocator.h"
#include "Checker.h"
#include "Logger.h"
#include "MacOS/MacOSString.h"
#include "MetalDevice.h"

namespace cube
{
    namespace gapi
    {
        MetalShader::MetalShader(MetalShaderCompileResult result, StringView warningMessage, StringView errorMessage) :
            mLibrary(result.library),
            mFunction(result.function)
        {
            mCreated = result.function != nil;
            mWarningMessage = warningMessage;
            mErrorMessage = errorMessage;
        }

        MetalShader::~MetalShader()
        {
        }
    } // namespace gapi
} // namespace cube
