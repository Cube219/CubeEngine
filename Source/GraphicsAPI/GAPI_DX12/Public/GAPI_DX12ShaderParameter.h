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

            virtual void UpdateShaderParameterListInfo(ShaderParameterListInfo& inOutParameterListInfo) const override;
            virtual void WriteParametersToGPUBuffer(SharedPtr<Buffer> buffer, const ShaderParameterListInfo& parameterListInfo, const void* pParameterList) const override;
            virtual const Vector<Vector<ShaderParameterReflection::Type>>& GetCompatibleShaderParameterReflectionTypeMap() const override { return mCompatibleShaderParameterReflectionTypeMap; }

            int GetMaxNumRegister() const { return mMaxNumRegister; }
            int GetMaxNumSpace() const { return mMaxNumSpace; }
            ID3D12RootSignature* GetRootSignature() const { return mRootSignature.Get(); }

        private:
            void InitializeCompatibleShaderParameterReflectionTypeMap();

            DX12Device& mDevice;

            int mMaxNumRegister;
            int mMaxNumSpace;

            ComPtr<ID3D12RootSignature> mRootSignature;

            Vector<Vector<ShaderParameterReflection::Type>> mCompatibleShaderParameterReflectionTypeMap;
        };
    } // namespace gapi
} // namespace cube
