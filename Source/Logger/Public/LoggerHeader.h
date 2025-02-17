#pragma once

#include "Types.h"

#ifdef CUBE_MODULE_LOGGER
#define CUBE_LOGGER_EXPORT __declspec(dllexport)
#else // CUBE_MODULE_LOGGER
#define CUBE_LOGGER_EXPORT __declspec(dllimport)
#endif // CUBE_MODULE_LOGGER
