#pragma once

#include "GAPIHeader.h"

namespace cube
{
    struct ShaderParameterInfo;

    namespace gapi
    {
        class Buffer;

        struct ShaderParametersAllocationInfo
        {
            Vector<Uint32> offsets;
            Uint32 totalBufferSize;
        };

        class ShaderParameterHelper
        {
        public:
            ShaderParameterHelper() = default;
            virtual ~ShaderParameterHelper() = default;
            
            virtual void UpdateShaderParameterInfo(Vector<ShaderParameterInfo>& inOutParameterInfos, Uint32& outTotalBufferSize) const = 0;
            virtual void WriteParametersToBuffer(SharedPtr<Buffer> buffer, const Vector<ShaderParameterInfo>& paramInfos, const void* pParameters) const = 0;
        };
    } // namespace gapi
} // namespace cube
