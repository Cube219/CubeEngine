#pragma once

#include "Defines.h"
#include "Types.h"

#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>

#ifdef CUBE_MODULE_GAPI_METAL
#define CUBE_METAL_EXPORT CUBE_DLL_EXPORT
#else // CUBE_MODULE_GAPI_METAL
#define CUBE_METAL_EXPORT CUBE_DLL_IMPORT
#endif // CUBE_MODULE_GAPI_METAL
