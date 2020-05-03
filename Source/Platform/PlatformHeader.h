#pragma once
#pragma warning(disable:4251)

#include "Utility/Types.h"

#ifdef PLATFORM_EXPORTS
#define PLATFORM_EXPORT __declspec(dllexport) 
#else // PLATFORM_EXPORTS
#define PLATFORM_EXPORT __declspec(dllimport) 
#endif // PLATFORM_EXPORTS
