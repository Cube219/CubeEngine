#pragma once
#pragma warning(disable:4251)

#include "Utility/Types.h"
#include "InputModuleFwd.h"

#ifdef INPUTMODULE_EXPORTS
#define INPUTMODULE_EXPORT __declspec(dllexport) 
#else // INPUTMODULE_EXPORTS
#define INPUTMODULE_EXPORT __declspec(dllimport) 
#endif // INPUTMODULE_EXPORTS
