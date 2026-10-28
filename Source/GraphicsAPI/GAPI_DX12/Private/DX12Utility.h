#pragma once

#include "DX12Header.h"

#include "Allocator/FrameAllocator.h"
#include "Checker.h"
#include "Logger.h"
#include "Platform/Windows/WindowsString.h"

namespace cube
{
#if CUBE_USE_CHECK

#define CHECK_HR(HR) \
    { \
        HRESULT res = HR; \
        if (res != S_OK) \
        { \
            cube::Checker::ProcessFailedCheckFormatting(__FILE__, __LINE__, CUBE_T(#HR)" == S_OK", CUBE_T("Failed to check HRESULT! (HRESULT: 0x{:0X})"), (unsigned long)(res)); \
            if (cube::Checker::IsDebuggerAttached()) \
            { \
                CUBE_DEBUG_BREAK \
            } \
        } \
    }

#else

#define CHECK_HR(HR) HR

#endif // CUBE_USE_CHECK

#define SET_DEBUG_NAME(object, name) \
    { \
        cube::FrameAllocationScope scope; \
        FrameWindowsString str = String_Convert<FrameWindowsString>(name); \
        if (!str.empty()) \
        { \
            object->SetPrivateData(WKPDID_D3DDebugObjectNameW, str.size() * sizeof(WindowsCharacter), str.c_str()); \
        } \
    }

#define SET_DEBUG_NAME_FORMAT(object, format, ...) \
    { \
        cube::FrameAllocationScope scope; \
        FrameWindowsString str = Format<FrameWindowsString>(WINDOWS_T(format), ##__VA_ARGS__); \
        object->SetPrivateData(WKPDID_D3DDebugObjectNameW, str.size() * sizeof(WindowsCharacter), str.c_str()); \
    }
} // namespace cube
