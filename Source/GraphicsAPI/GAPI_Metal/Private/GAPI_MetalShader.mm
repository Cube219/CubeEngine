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
        MetalShader::MetalShader(const MetalShaderCompileResult& result, StringView warningMessage, StringView errorMessage, Vector<platform::FilePath>&& dependencyFilePaths) :
            mLibrary(result.library),
            mFunction(result.function)
        {
            mCreated = result.function != nil;
            mWarningMessage = warningMessage;
            mErrorMessage = errorMessage;
            mDependencyFilePaths = std::move(dependencyFilePaths);

            mReflection = result.reflection;
            mThreadGroupSizeX = mReflection.threadGroupSizeX;
            mThreadGroupSizeY = mReflection.threadGroupSizeY;
            mThreadGroupSizeZ = mReflection.threadGroupSizeZ;
        }

        MetalShader::~MetalShader()
        {
            if (mFunction)
            {
                [mFunction release];
            }
            if (mLibrary)
            {
                [mLibrary release];
            }
        }
    } // namespace gapi
} // namespace cube
