#pragma once

#include "MetalHeader.h"

#include "GAPI_ShaderParameter.h"

namespace cube
{
    namespace gapi
    {
        class MetalShaderParameterHelper : public ShaderParameterHelper
        {
        public:
            MetalShaderParameterHelper() {}
            virtual ~MetalShaderParameterHelper() {}

            virtual void UpdateShaderParameterInfo(Vector<ShaderParameterInfo>& inOutParameterInfos, Uint32& outTotalBufferSize) const override {}
            virtual void WriteParametersToBuffer(SharedPtr<Buffer> buffer, const Vector<ShaderParameterInfo>& paramInfos, const void* pParameters) const override {}
        };
    } // namespace gapi
} // namespace cube
