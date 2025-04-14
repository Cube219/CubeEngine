#pragma once

#include "Defines.h"
#include "Types.h"

#ifdef CUBE_MODULE_CORE
#define CUBE_CORE_EXPORT CUBE_DLL_EXPORT
#else
#define CUBE_CORE_EXPORT CUBE_DLL_IMPORT
#endif // CUBE_MODULE_CORE
