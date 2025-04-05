#ifdef CUBE_PLATFORM_MACOS

#include "MacOS/MacOSDLib.h"

#include "Checker.h"

namespace cube
{
    namespace platform
    {
        DLIB_CLASS_DEFINITIONS(MacOSDLib)

        MacOSDLib::MacOSDLib(StringView path)
        {
            NOT_IMPLEMENTED();
        }

        MacOSDLib::~MacOSDLib()
        {
            NOT_IMPLEMENTED();
        }

        void* MacOSDLib::GetFunctionImpl(StringView name)
        {
            NOT_IMPLEMENTED();
            return nullptr;
        }
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS
