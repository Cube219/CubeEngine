#include "GAPI_DX12Resource.h"

namespace cube
{
    namespace gapi
    {
        D3D12_RESOURCE_STATES ConvertToDX12ResourceStates(ResourceStateFlags stateFlags)
        {
            D3D12_RESOURCE_STATES states = D3D12_RESOURCE_STATE_COMMON;

            if (stateFlags.IsSet(ResourceStateFlag::Vertex))
            {
                states |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
            }
            if (stateFlags.IsSet(ResourceStateFlag::Index))
            {
                states |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
            }
            if (stateFlags.IsSet(ResourceStateFlag::RenderTarget))
            {
                states |= D3D12_RESOURCE_STATE_RENDER_TARGET;
            }
            if (stateFlags.IsSet(ResourceStateFlag::SRV_Pixel))
            {
                states |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
            }
            if (stateFlags.IsSet(ResourceStateFlag::SRV_NonPixel))
            {
                states |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            }
            if (stateFlags.IsSet(ResourceStateFlag::UAV))
            {
                states |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            }
            if (stateFlags.IsSet(ResourceStateFlag::DepthRead))
            {
                states |= D3D12_RESOURCE_STATE_DEPTH_READ;
            }
            if (stateFlags.IsSet(ResourceStateFlag::DepthWrite))
            {
                states |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
            }
            if (stateFlags.IsSet(ResourceStateFlag::IndirectArgs))
            {
                states |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
            }
            if (stateFlags.IsSet(ResourceStateFlag::CopySrc))
            {
                states |= D3D12_RESOURCE_STATE_COPY_SOURCE;
            }
            if (stateFlags.IsSet(ResourceStateFlag::CopyDst))
            {
                states |= D3D12_RESOURCE_STATE_COPY_DEST;
            }
            if (stateFlags.IsSet(ResourceStateFlag::ResolveSrc))
            {
                states |= D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
            }
            if (stateFlags.IsSet(ResourceStateFlag::ResolveDst))
            {
                states |= D3D12_RESOURCE_STATE_RESOLVE_DEST;
            }
            if (stateFlags.IsSet(ResourceStateFlag::Present))
            {
                states |= D3D12_RESOURCE_STATE_PRESENT;
            }

            return states;
        }
    } // namespace gapi
} // namespace cube
