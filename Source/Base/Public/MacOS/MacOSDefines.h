#pragma once

// XCode library doesn't support unicode characters correctly
//     - Ex) locale
#define CUBE_SUPPORT_UNICODE_CHARACTER 0

#define CUBE_DLL_HIDDEN __attribute__((visibility("hidden")))

#define CUBE_VECTOR_USE_SSE 0
#define CUBE_VECTOR_USE_NEON 1

#ifdef CUBE_DEBUG

#define CUBE_DEBUG_BREAK __builtin_debugtrap();

#else

#define CUBE_DEBUG_BREAK

#endif
