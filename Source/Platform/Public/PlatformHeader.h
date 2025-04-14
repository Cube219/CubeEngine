#pragma once

#include "CubeString.h"
#include "Defines.h"
#include "Types.h"

#ifdef CUBE_MODULE_PLATFORM
#define CUBE_PLATFORM_EXPORT CUBE_DLL_EXPORT
#else // CUBE_MODULE_PLATFORM
#define CUBE_PLATFORM_EXPORT CUBE_DLL_IMPORT
#endif // CUBE_MODULE_PLATFORM
