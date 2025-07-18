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
            // TODO: Use version 1.1?
            FrameVector<D3D12_ROOT_PARAMETER> parameters;
            parameters.reserve(info.numShaderVariablesConstantBuffer);

            for (Uint32 i = 0; i < info.numShaderVariablesConstantBuffer; ++i)
            {
                // const ShaderVariableConstantBuffer& cb = info.shaderVariablesConstantBuffer[i];

                parameters.push_back({
                    .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                    .Descriptor = {
                        .ShaderRegister = i,
                        .RegisterSpace = 0
                    },
                    .ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL // TODO
                });
            }

            const D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {
                .NumParameters = static_cast<Uint32>(parameters.size()),
                .pParameters = parameters.data(),
                .NumStaticSamplers = 0,
                .pStaticSamplers = nullptr,
                .Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
                    | D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
                    | D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED
            };

            ComPtr<ID3DBlob> signatureBlob;
            ComPtr<ID3DBlob> errorBlob;
            CHECK_HR(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob));
            CHECK_HR(device.GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));
            SET_DEBUG_NAME(mRootSignature, info.debugName);
        }

        DX12ShaderVariablesLayout::~DX12ShaderVariablesLayout()
        {
            mRootSignature = nullptr;
        }
    } // namespace gapi
} // namespace cube
