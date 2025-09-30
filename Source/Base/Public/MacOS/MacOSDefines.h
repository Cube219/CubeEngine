#pragma once

// XCode library doesn't support unicode characters correctly
//     - Ex) locale
#define CUBE_SUPPORT_UNICODE_CHARACTER 0

// TODO: Support SSE
#define CUBE_VECTOR_USE_SSE 0

#ifdef CUBE_DEBUG

#define CUBE_DEBUG_BREAK __builtin_debugtrap();

#else

#define CUBE_DEBUG_BREAK

#endif
