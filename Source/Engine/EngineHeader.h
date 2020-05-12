#pragma once
#pragma warning(disable:4251)

#include "Utility/Types.h"
#include "EngineFwd.h"

#ifdef ENGINE_EXPORTS
#define ENGINE_EXPORT __declspec(dllexport) 
#else // ENGINE_EXPORTS
#define ENGINE_EXPORT __declspec(dllimport) 
#endif // ENGINE_EXPORTS
