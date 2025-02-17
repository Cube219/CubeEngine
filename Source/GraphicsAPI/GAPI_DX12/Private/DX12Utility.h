#pragma once

#include "DX12Header.h"

#include "Checker.h"
#include "Logger.h"

namespace cube
{
#define CHECK_HR(HR) \
    if ((HR) != S_OK) \
    { \
        cube::Checker::ProcessFailedCheckFormatting(__FILE__, __LINE__, CUBE_T(#HR)" == S_OK", CUBE_T("Failed to check HRESULT! (HRESULT: 0x{:0X})"), (unsigned long)(HR)); \
    }
} // namespace cube
