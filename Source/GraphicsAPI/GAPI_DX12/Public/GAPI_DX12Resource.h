#pragma once

#include "DX12Header.h"

#include "GAPI_Resource.h"

namespace cube
{
	namespace gapi
	{
        D3D12_RESOURCE_STATES ConvertToDX12ResourceStates(ResourceStateFlags stateFlags);
	} // namespace gapi
} // namespace cube
