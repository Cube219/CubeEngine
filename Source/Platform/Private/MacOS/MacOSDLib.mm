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
        DLIB_CLASS_DEFINITIONS(MacOSDLib)

        MacOSDLib::MacOSDLib(StringView path)
        {
            // TODO: Separate file name
            MacOSString u8Path = Format<MacOSString>("lib{}.dylib", path);
            mHandle = dlopen(u8Path.data(), RTLD_NOW);

            if (mHandle == nullptr)
            {
                CUBE_LOG(LogType::Warning, MacOSDLib, "Failed to load a DLib. (lib{0}.dylib)", path);
            }
        }

        MacOSDLib::~MacOSDLib()
        {
            if (mHandle)
            {
                dlclose(mHandle);
            }
        }

        void* MacOSDLib::GetFunctionImpl(StringView name)
        {
            if (!mHandle)
            {
                return nullptr;
            }

            MacOSString macName;
            String_ConvertAndAppend(macName, name);
            void *pFunction = dlsym(mHandle, macName.c_str());
            if (pFunction == nullptr)
            {
                CUBE_LOG(LogType::Warning, MacOSDLib, "Failed to get the function({0}).", name);
            }

            return pFunction;
        }
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS
