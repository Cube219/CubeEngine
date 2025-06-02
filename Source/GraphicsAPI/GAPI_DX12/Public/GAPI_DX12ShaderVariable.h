#pragma once

#include "DX12Header.h"

#include "GAPI_ShaderVariable.h"

#include "DX12APIObject.h"

namespace cube
{
    class DX12Device;

    namespace gapi
    {
        class DX12ShaderVariablesLayout : public ShaderVariablesLayout, public DX12APIObject
        {
        public:
            DX12ShaderVariablesLayout(DX12Device& device, const ShaderVariablesLayoutCreateInfo& info);
            virtual ~DX12ShaderVariablesLayout();

            ID3D12RootSignature* GetRootSignature() const { return mRootSignature.Get(); }

        private:
            ComPtr<ID3D12RootSignature> mRootSignature;
        };
    } // namespace gapi
} // namespace cube
