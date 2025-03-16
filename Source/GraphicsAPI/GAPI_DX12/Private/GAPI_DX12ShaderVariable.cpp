#include "GAPI_DX12ShaderVariable.h"

#include "DX12Device.h"
#include "DX12Utility.h"

namespace cube
{
    namespace gapi
    {
        DX12ShaderVariablesLayout::DX12ShaderVariablesLayout(DX12Device& device, const ShaderVariablesLayoutCreateInfo& info)
        {
            // Currently create null root signature
            D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {
                .NumParameters = 0,
                .pParameters = nullptr,
                .NumStaticSamplers = 0,
                .pStaticSamplers = nullptr,
                .Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
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
