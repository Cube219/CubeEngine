#include "GAPI_DX12Sampler.h"

#include "DX12Device.h"

namespace cube
{
    namespace gapi
    {
        D3D12_FILTER_TYPE ConvertToD3D12FilterType(SamplerFilterType type)
        {
            switch (type)
            {
            case SamplerFilterType::Point:
                return D3D12_FILTER_TYPE_POINT;
            case SamplerFilterType::Linear:
                return D3D12_FILTER_TYPE_LINEAR;
            default:
                NOT_IMPLEMENTED();
                break;
            }   
            return D3D12_FILTER_TYPE_LINEAR;
        }

        D3D12_TEXTURE_ADDRESS_MODE ConvertToD3D12TextureAddressMode(SamplerAddressMode mode)
        {
            switch (mode)
            {
            case SamplerAddressMode::Warp:
                return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            case SamplerAddressMode::Mirror:
                return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            case SamplerAddressMode::Clamp:
                return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            case SamplerAddressMode::Border:
                return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            case SamplerAddressMode::MirrorOnce:
                return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
            default:
                NOT_IMPLEMENTED();
                break;
            }
            return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        }

        DX12Sampler::DX12Sampler(DX12Device& device, const SamplerCreateInfo& info) :
            mDevice(device)
        {
            D3D12_FILTER filter;
            if (info.useAnisotropy)
            {
                filter = D3D12_ENCODE_ANISOTROPIC_FILTER(D3D12_FILTER_REDUCTION_TYPE_STANDARD);
            }
            else
            {
                D3D12_FILTER_TYPE minFilter = ConvertToD3D12FilterType(info.minFilter);
                D3D12_FILTER_TYPE magFilter = ConvertToD3D12FilterType(info.magFilter);
                D3D12_FILTER_TYPE mipFilter = ConvertToD3D12FilterType(info.mipFilter);

                filter = D3D12_ENCODE_BASIC_FILTER(minFilter, magFilter, mipFilter, D3D12_FILTER_REDUCTION_TYPE_STANDARD);
            }

            D3D12_SAMPLER_DESC samplerDesc = {
                .Filter = filter,
                .AddressU = ConvertToD3D12TextureAddressMode(info.addressU),
                .AddressV = ConvertToD3D12TextureAddressMode(info.addressV),
                .AddressW = ConvertToD3D12TextureAddressMode(info.addressW),
                .MipLODBias = info.mipLodBias,
                .MaxAnisotropy = info.maxAnisotropy,
                .ComparisonFunc = D3D12_COMPARISON_FUNC_NONE,
                .BorderColor = {
                    info.borderColor[0],
                    info.borderColor[1],
                    info.borderColor[2],
                    info.borderColor[3],
                },
                .MinLOD = info.minLod,
                .MaxLOD = info.maxLod
            };

            mDescriptor = device.GetDescriptorManager().GetSamplerHeap().AllocateCPU();

            device.GetDevice()->CreateSampler(&samplerDesc, mDescriptor.handle);
        }

        DX12Sampler::~DX12Sampler()
        {
            mDevice.GetDescriptorManager().GetSamplerHeap().FreeCPU(mDescriptor);
        }
    } // namespace gapi
} // namespace cube
