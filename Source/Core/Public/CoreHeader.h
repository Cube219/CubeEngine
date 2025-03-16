#pragma once

#include "Types.h"

#ifdef CUBE_MODULE_CORE
#define CUBE_CORE_EXPORT __declspec(dllexport)
#else
#define CUBE_CORE_EXPORT __declspec(dllimport)
#endif // CUBE_MODULE_CORE
