#ifdef CUBE_PLATFORM_WINDOWS

#include "Windows/WindowsDLib.h"

#include "Checker.h"
#include "Windows/WindowsString.h"

namespace cube
{
    namespace platform
    {
        DLIB_CLASS_DEFINITIONS(WindowsDLib)

        WindowsDLib::WindowsDLib(StringView path)
        {
            WindowsString pathWithExtension;
            String_ConvertAndAppend(pathWithExtension, path);
            pathWithExtension.append(L".dll");

            mDLib = LoadLibrary(pathWithExtension.c_str());
            if (mDLib == nullptr)
            {
                CUBE_LOG(Warning, WindowsDLib, "Failed to load a DLib. ({0}.dll) (ErrorCode: {1})", path, GetLastError());
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

        void* WindowsDLib::GetFunctionImpl(StringView name)
        {
            if (!mDLib)
            {
                return nullptr;
            }

            AnsiString aName;
            String_ConvertAndAppend(aName, name);
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
