#pragma once
#pragma warning(disable:4251)

#include "Utility/Types.h"
#include "CoreFwd.h"

#ifdef CORE_EXPORTS
#define CORE_EXPORT __declspec(dllexport) 
#else // CORE_EXPORTS
#define CORE_EXPORT __declspec(dllimport) 
#endif // CORE_EXPORTS
