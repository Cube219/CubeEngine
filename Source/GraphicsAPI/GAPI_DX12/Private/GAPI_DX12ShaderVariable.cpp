#include "GAPI_DX12ShaderVariable.h"

#include "DX12Device.h"
#include "DX12Utility.h"
#include "Allocator/FrameAllocator.h"

namespace cube
{
    namespace gapi
    {
        DX12ShaderVariablesLayout::DX12ShaderVariablesLayout(DX12Device& device, const ShaderVariablesLayoutCreateInfo& info)
        {
            FrameVector<D3D12_ROOT_PARAMETER1> parameters;

            const DX12ShaderParameterHelper& shaderParameterHelper = device.GetShaderParameterHelper();
            const Uint32 maxNumRegister = shaderParameterHelper.GetMaxNumRegister();
            const Uint32 maxNumSpace = shaderParameterHelper.GetMaxNumSpace();
            parameters.reserve(maxNumRegister * maxNumSpace);
            for (Uint32 space = 0; space < maxNumSpace; ++space)
            {
                for (Uint32 reg = 0; reg < maxNumRegister; ++reg)
                {
                    parameters.push_back({
                        .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                        .Descriptor = {
                            .ShaderRegister = reg,
                            .RegisterSpace = space,
                            .Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC
                        },
                        .ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
                    });
                }
            }

            const D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc = {
                .Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
                .Desc_1_1 = {
                    .NumParameters = static_cast<Uint32>(parameters.size()),
                    .pParameters = parameters.data(),
                    .NumStaticSamplers = 0,
                    .pStaticSamplers = nullptr,
                    .Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
                        | D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
                        | D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED
                }
            };

            ComPtr<ID3DBlob> signatureBlob;
            ComPtr<ID3DBlob> errorBlob;
            CHECK_HR(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, device.GetMaxRootSignatureVersion(), &signatureBlob, &errorBlob));
            CHECK_HR(device.GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));
            SET_DEBUG_NAME(mRootSignature, info.debugName);
        }

        DX12ShaderVariablesLayout::~DX12ShaderVariablesLayout()
        {
            mRootSignature = nullptr;
        }
    } // namespace gapi
} // namespace cube
