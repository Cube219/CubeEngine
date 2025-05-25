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
        MetalShader::MetalShader(MetalShaderCompileResult result) :
            mLibrary(result.library),
            mFunction(result.function)
        {
        }

        MetalShader::~MetalShader()
        {
        }
    } // namespace gapi
} // namespace cube
