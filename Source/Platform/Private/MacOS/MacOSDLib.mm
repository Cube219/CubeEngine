#ifdef CUBE_PLATFORM_MACOS

#include "MacOS/MacOSDLib.h"

#include <dlfcn.h>
#include <Foundation/Foundation.h>

#include "Checker.h"
#include "CubeString.h"
#include "Format.h"
#include "MacOS/MacOSString.h"

namespace cube
{
    namespace platform
    {
        MacOSDLib::MacOSDLib(const FilePath& path)
        {
            NSString* fullPath = [NSString stringWithFormat:@"lib%@.dylib", path.GetNativePath()];
            mHandle = dlopen([fullPath UTF8String], RTLD_NOW);

            if (mHandle == nullptr)
            {
                CUBE_LOG(Warning, MacOSDLib, "Failed to load a DLib. ({})", [fullPath UTF8String]);
            }
        }

        MacOSDLib::~MacOSDLib()
        {
            if (mHandle)
            {
                dlclose(mHandle);
            }
        }

        void* MacOSDLib::GetFunction(StringView name)
        {
            if (!mHandle)
            {
                return nullptr;
            }

            MacOSString macName = String_Convert<MacOSString>(name);
            void *pFunction = dlsym(mHandle, macName.c_str());
            if (pFunction == nullptr)
            {
                CUBE_LOG(Warning, MacOSDLib, "Failed to get the function({0}).", name);
            }

            return pFunction;
        }
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS
