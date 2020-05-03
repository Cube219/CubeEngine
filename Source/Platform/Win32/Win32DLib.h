#pragma once

#ifdef _WIN32

#include "../PlatformHeader.h"

#include <Windows.h>

#include "../DLib.h"

namespace cube
{
    namespace platform
    {
        class PLATFORM_EXPORT Win32DLib : public DLib
        {
        public:
            Win32DLib(StringView path);
            virtual ~Win32DLib();

            void* GetFunctionImpl(StringView name);

        private:
            HMODULE mDLib;
        };
        DLIB_DEFINITIONS(Win32DLib);
    } // namespace platform
} // namespace cube

#endif // _WIN32
