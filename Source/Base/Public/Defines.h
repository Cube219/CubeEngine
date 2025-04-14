#pragma once

// Global defines

#if defined(NDEBUG)
#define CUBE_DEBUG 0
#else
#define CUBE_DEBUG 1
#endif

// Platform-dependent defines
#if CUBE_PLATFORM_WINDOWS
#include "Windows/WindowsDefines.h"
#elif CUBE_PLATFORM_MACOS
#include "MacOS/MacOSDefines.h"
#endif

#ifndef FORCE_INLINE
#define FORCE_INLINE
#endif

#ifndef CUBE_DLL_EXPORT
#define CUBE_DLL_EXPORT
#endif

#ifndef CUBE_DLL_IMPORT
#define CUBE_DLL_IMPORT
#endif
