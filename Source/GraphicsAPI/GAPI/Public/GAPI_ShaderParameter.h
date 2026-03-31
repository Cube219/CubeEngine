#pragma once

#include "GAPIHeader.h"

#include "GAPI_ShaderReflection.h"

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
            virtual void WriteParametersToGPUBuffer(SharedPtr<Buffer> buffer, const Vector<ShaderParameterInfo>& paramInfos, const void* pParameters) const = 0;

            virtual const Vector<Vector<ShaderParameterReflection::Type>>& GetCompatibleShaderParameterReflectionTypeMap() const = 0;
        };
    } // namespace gapi
} // namespace cube
