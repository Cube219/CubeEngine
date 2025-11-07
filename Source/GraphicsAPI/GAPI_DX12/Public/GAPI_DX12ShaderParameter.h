#pragma once

#include "DX12Header.h"

#include "GAPI_ShaderParameter.h"

#include "DX12APIObject.h"
#include "GAPI_DX12Buffer.h"

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

            virtual void UpdateShaderParameterInfo(Vector<ShaderParameterInfo>& inOutParameterInfos, Uint32& outTotalBufferSize) const override;
            virtual void WriteParametersToBuffer(SharedPtr<Buffer> buffer, const Vector<ShaderParameterInfo>& paramInfos, const void* pParameters) const override;
        };
    } // namespace gapi
} // namespace cube
