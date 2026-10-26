#pragma once

#ifndef CUBE_VECTOR_USE_AVX2
#define CUBE_VECTOR_USE_AVX2 0
#endif

#if CUBE_VECTOR_USE_AVX2
#define CUBE_VECTOR_USE_SSE 0
#else
#define CUBE_VECTOR_USE_SSE 1
#endif

#define FORCE_INLINE __forceinline

#define CUBE_DLL_EXPORT __declspec(dllexport)
#define CUBE_DLL_IMPORT __declspec(dllimport)

#ifdef CUBE_DEBUG

// Include debug related headers to use CUBE_DEBUG_BREAK in other source files
#include <Windows.h>
#define CUBE_DEBUG_BREAK DebugBreak();

#else

#define CUBE_DEBUG_BREAK

#endif
