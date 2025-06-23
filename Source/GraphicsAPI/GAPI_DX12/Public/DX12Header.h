#pragma once

#include "Defines.h"
#include "Types.h"

#include <d3dx12/d3dx12.h> // Should be included before other dx12 headers
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

#ifdef CUBE_MODULE_GAPI_DX12
#define CUBE_DX12_EXPORT CUBE_DLL_EXPORT
#else // CUBE_MODULE_GAPI_DX12
#define CUBE_DX12_EXPORT CUBE_DLL_IMPORT
#endif // CUBE_MODULE_GAPI_DX12
