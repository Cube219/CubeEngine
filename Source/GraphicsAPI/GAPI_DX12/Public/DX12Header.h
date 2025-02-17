#pragma once

#include "Types.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

#ifdef CUBE_MODULE_GAPI_DX12
#define CUBE_DX12_EXPORT __declspec(dllexport)
#else // CUBE_MODULE_GAPI_DX12
#define CUBE_DX12_EXPORT __declspec(dllimport)
#endif // CUBE_MODULE_GAPI_DX12
