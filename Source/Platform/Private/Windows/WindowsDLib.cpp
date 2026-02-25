#ifdef CUBE_PLATFORM_WINDOWS

#include "Windows/WindowsDLib.h"

#include "Checker.h"
#include "Windows/WindowsString.h"

namespace cube
{
    namespace platform
    {
        WindowsDLib::WindowsDLib(const FilePath& path)
        {
            WindowsString fullPath = Format<WindowsString>(WINDOWS_T("{0}.dll"), path.GetNativePath());

            mDLib = LoadLibrary(fullPath.c_str());
            if (mDLib == nullptr)
            {
                CUBE_LOG(Warning, WindowsDLib, "Failed to load a DLib. ({0}.dll) (ErrorCode: {1})", path.ToString(), GetLastError());
            }
        }

        WindowsDLib::~WindowsDLib()
        {
            if (mDLib)
            {
                BOOL r = FreeLibrary(mDLib);
                CHECK_FORMAT(r, "Failed to unload the DLib. (ErrorCode: {0})", GetLastError());
            }
        }

        void* WindowsDLib::GetFunction(StringView name)
        {
            if (!mDLib)
            {
                return nullptr;
            }

            AnsiString aName = String_Convert<AnsiString>(name);
            auto pFunction = GetProcAddress(mDLib, aName.c_str());
            if (pFunction == nullptr)
            {
                CUBE_LOG(Warning, WindowsDLib, "Failed to get the function({0}). (ErrorCode: {1})", name, GetLastError());
            }

            return reinterpret_cast<void*>(pFunction);
        }
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_WINDOWS
