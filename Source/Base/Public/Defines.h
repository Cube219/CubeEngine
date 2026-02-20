#pragma once

// Global defines

#if defined(NDEBUG)
#define CUBE_DEBUG 0
#else
#define CUBE_DEBUG 1
#endif

#define CUBE_MACRO_JOIN(a, b) CUBE_MACRO_JOIN2(a, b)
#define CUBE_MACRO_JOIN2(a, b) a##b

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

#ifndef CUBE_DLL_HIDDEN
#define CUBE_DLL_HIDDEN
#endif

#ifndef CUBE_DEBUG_BREAK
#define CUBE_DEBUG_BREAK
#endif
