#pragma once

#include "CubeString.h"
#include "Types.h"

#ifdef CUBE_MODULE_PLATFORM
#define CUBE_PLATFORM_EXPORT __declspec(dllexport)
#else // CUBE_MODULE_PLATFORM
#define CUBE_PLATFORM_EXPORT __declspec(dllimport)
#endif // CUBE_MODULE_PLATFORM
