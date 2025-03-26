#pragma once

#include "DX12Header.h"

#include "Allocator/FrameAllocator.h"
#include "Checker.h"
#include "Logger.h"

namespace cube
{
#define CHECK_HR(HR) \
    if ((HR) != S_OK) \
    { \
        cube::Checker::ProcessFailedCheckFormatting(__FILE__, __LINE__, CUBE_T(#HR)" == S_OK", CUBE_T("Failed to check HRESULT! (HRESULT: 0x{:0X})"), (unsigned long)(HR)); \
    }

#define SET_DEBUG_NAME(object, pName) \
    { \
        AnsiStringView str(pName); \
        object->SetPrivateData(WKPDID_D3DDebugObjectName, str.size(), str.data()); \
    }

#define SET_DEBUG_NAME_FORMAT(object, format, ...) \
    { \
        FrameAnsiString str = Format<FrameAnsiString>(format, ##__VA_ARGS__); \
        object->SetPrivateData(WKPDID_D3DDebugObjectName, str.size(), str.c_str()); \
    }
} // namespace cube
