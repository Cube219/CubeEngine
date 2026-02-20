#pragma once

#include "DX12Header.h"

#include "GAPI_ShaderParameter.h"

namespace cube
{
    class DX12Device;

    namespace gapi
    {
        class DX12ShaderParameterHelper : public ShaderParameterHelper
        {
        public:
            DX12ShaderParameterHelper(DX12Device& device);
            virtual ~DX12ShaderParameterHelper();

            void Initialize();
            void Shutdown();

            virtual void UpdateShaderParameterInfo(Vector<ShaderParameterInfo>& inOutParameterInfos, Uint32& outTotalBufferSize) const override;
            virtual void WriteParametersToBuffer(SharedPtr<Buffer> buffer, const Vector<ShaderParameterInfo>& paramInfos, const void* pParameters) const override;

            int GetMaxNumRegister() const { return mMaxNumRegister; }
            int GetMaxNumSpace() const { return mMaxNumSpace; }
            ID3D12RootSignature* GetRootSignature() const { return mRootSignature.Get(); }

        private:
            DX12Device& mDevice;

            int mMaxNumRegister;
            int mMaxNumSpace;

            ComPtr<ID3D12RootSignature> mRootSignature;
        };
    } // namespace gapi
} // namespace cube
