#include "GAPI_DX12Resource.h"

#include "Checker.h"

namespace cube
{
    namespace gapi
    {
        D3D12_BARRIER_SYNC ConvertToDX12ResourceSyncFlags(ResourceSyncFlags syncFlags)
        {
            D3D12_BARRIER_SYNC sync = D3D12_BARRIER_SYNC_NONE;

            if (syncFlags.IsSet(ResourceSyncFlag::All))
            {
                return D3D12_BARRIER_SYNC_ALL;
            }
            if (syncFlags.IsSet(ResourceSyncFlag::Index))
            {
                sync |= D3D12_BARRIER_SYNC_INDEX_INPUT;
            }
            if (syncFlags.IsSet(ResourceSyncFlag::Vertex))
            {
                sync |= D3D12_BARRIER_SYNC_VERTEX_SHADING;
            }
            if (syncFlags.IsSet(ResourceSyncFlag::Pixel))
            {
                sync |= D3D12_BARRIER_SYNC_PIXEL_SHADING;
            }
            if (syncFlags.IsSet(ResourceSyncFlag::DepthStencil))
            {
                sync |= D3D12_BARRIER_SYNC_DEPTH_STENCIL;
            }
            if (syncFlags.IsSet(ResourceSyncFlag::RenderTarget))
            {
                sync |= D3D12_BARRIER_SYNC_RENDER_TARGET;
            }
            if (syncFlags.IsSet(ResourceSyncFlag::Compute))
            {
                sync |= D3D12_BARRIER_SYNC_COMPUTE_SHADING;
            }
            if (syncFlags.IsSet(ResourceSyncFlag::Copy))
            {
                sync |= D3D12_BARRIER_SYNC_COPY;
            }
            if (syncFlags.IsSet(ResourceSyncFlag::Resolve))
            {
                sync |= D3D12_BARRIER_SYNC_RESOLVE;
            }
            if (syncFlags.IsSet(ResourceSyncFlag::ExecuteIndirect))
            {
                sync |= D3D12_BARRIER_SYNC_EXECUTE_INDIRECT;
            }
            if (syncFlags.IsSet(ResourceSyncFlag::ClearUAV))
            {
                sync |= D3D12_BARRIER_SYNC_CLEAR_UNORDERED_ACCESS_VIEW;
            }

            return sync;
        }
        
        D3D12_BARRIER_ACCESS ConvertToDX12ResourceAccessFlags(ResourceAccessFlags accessFlags)
        {
            D3D12_BARRIER_ACCESS access = D3D12_BARRIER_ACCESS_COMMON;

            if (accessFlags == ResourceAccessFlag::Common)
            {
                return D3D12_BARRIER_ACCESS_COMMON;
            }
            if (accessFlags.IsSet(ResourceAccessFlag::NoAccess))
            {
                return D3D12_BARRIER_ACCESS_NO_ACCESS;
            }
            if (accessFlags.IsSet(ResourceAccessFlag::ConstantBuffer))
            {
                access |= D3D12_BARRIER_ACCESS_CONSTANT_BUFFER;
            }
            if (accessFlags.IsSet(ResourceAccessFlag::VertexBuffer))
            {
                access |= D3D12_BARRIER_ACCESS_VERTEX_BUFFER;
            }
            if (accessFlags.IsSet(ResourceAccessFlag::IndexBuffer))
            {
                access |= D3D12_BARRIER_ACCESS_INDEX_BUFFER;
            }
            if (accessFlags.IsSet(ResourceAccessFlag::RenderTarget))
            {
                access |= D3D12_BARRIER_ACCESS_RENDER_TARGET;
            }
            if (accessFlags.IsSet(ResourceAccessFlag::SRV))
            {
                access |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
            }
            if (accessFlags.IsSet(ResourceAccessFlag::UAV))
            {
                access |= D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
            }
            if (accessFlags.IsSet(ResourceAccessFlag::DepthStencilRead))
            {
                access |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;
            }
            if (accessFlags.IsSet(ResourceAccessFlag::DepthStencilWrite))
            {
                access |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;
            }
            if (accessFlags.IsSet(ResourceAccessFlag::CopySrc))
            {
                access |= D3D12_BARRIER_ACCESS_COPY_SOURCE;
            }
            if (accessFlags.IsSet(ResourceAccessFlag::CopyDst))
            {
                access |= D3D12_BARRIER_ACCESS_COPY_DEST;
            }
            if (accessFlags.IsSet(ResourceAccessFlag::ResolveSrc))
            {
                access |= D3D12_BARRIER_ACCESS_RESOLVE_SOURCE;
            }
            if (accessFlags.IsSet(ResourceAccessFlag::ResolveDst))
            {
                access |= D3D12_BARRIER_ACCESS_RESOLVE_DEST;
            }

            return access;
        }

        D3D12_BARRIER_LAYOUT ConvertToDX12ResourceLayout(ResourceLayout layout)
        {
            switch (layout)
            {
            case ResourceLayout::Undefined:
                return D3D12_BARRIER_LAYOUT_UNDEFINED;
            case ResourceLayout::Common:
                return D3D12_BARRIER_LAYOUT_COMMON;
            case ResourceLayout::Common_Direct:
                return D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COMMON;
            case ResourceLayout::Common_Async:
                return D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COMMON;
            case ResourceLayout::Present:
                return D3D12_BARRIER_LAYOUT_PRESENT;
            case ResourceLayout::GenericRead:
                return D3D12_BARRIER_LAYOUT_GENERIC_READ;
            case ResourceLayout::GenericRead_Direct:
                return D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_GENERIC_READ;
            case ResourceLayout::GenericRead_Async:
                return D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_GENERIC_READ;
            case ResourceLayout::RenderTarget:
                return D3D12_BARRIER_LAYOUT_RENDER_TARGET;
            case ResourceLayout::SRV:
                return D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
            case ResourceLayout::SRV_Direct:
                return D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_SHADER_RESOURCE;
            case ResourceLayout::SRV_Async:
                return D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_SHADER_RESOURCE;
            case ResourceLayout::UAV:
                return D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
            case ResourceLayout::UAV_Direct:
                return D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_UNORDERED_ACCESS;
            case ResourceLayout::UAV_Async:
                return D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_UNORDERED_ACCESS;
            case ResourceLayout::DepthStencilRead:
                return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
            case ResourceLayout::DepthStencilWrite:
                return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
            case ResourceLayout::CopySrc:
                return D3D12_BARRIER_LAYOUT_COPY_SOURCE;
            case ResourceLayout::CopySrc_Direct:
                return D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COPY_SOURCE;
            case ResourceLayout::CopySrc_Async:
                return D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COPY_SOURCE;
            case ResourceLayout::CopyDst:
                return D3D12_BARRIER_LAYOUT_COPY_DEST;
            case ResourceLayout::CopyDst_Direct:
                return D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COPY_DEST;
            case ResourceLayout::CopyDst_Async:
                return D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COPY_DEST;
            case ResourceLayout::ResolveSrc:
                return D3D12_BARRIER_LAYOUT_RESOLVE_SOURCE;
            case ResourceLayout::ResolveDst:
                return D3D12_BARRIER_LAYOUT_RESOLVE_DEST;
            default:
                NOT_IMPLEMENTED();
            }

            return D3D12_BARRIER_LAYOUT_COMMON;
        }
    } // namespace gapi
} // namespace cube
