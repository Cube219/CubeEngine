#pragma once

#ifdef CUBE_PLATFORM_WINDOWS

#include "PlatformHeader.h"
#include "DLib.h"

#include <Windows.h>

namespace cube
{
    namespace platform
    {
        class WindowsDLib : public DLib
        {
        public:
            WindowsDLib(StringView path);
            ~WindowsDLib();

            void* GetFunctionImpl(StringView name);

            HMODULE GetModule() const { return mDLib; }

        private:
            HMODULE mDLib;
        };
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_WINDOWS
