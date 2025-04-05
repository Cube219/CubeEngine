#pragma once

#ifdef CUBE_PLATFORM_MACOS

#include "PlatformHeader.h"
#include "DLib.h"

namespace cube
{
    namespace platform
    {
        class MacOSDLib : public DLib
        {
        public:
            MacOSDLib(StringView path);
            ~MacOSDLib();

            void* GetFunctionImpl(StringView name);
        };
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS
