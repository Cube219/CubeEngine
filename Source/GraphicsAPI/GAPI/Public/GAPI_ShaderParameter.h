#pragma once

#include "GAPIHeader.h"

#include "GAPI_ShaderReflection.h"

namespace cube
{
    struct ShaderParametersInfo;

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
            
            virtual void UpdateShaderParametersInfo(ShaderParametersInfo& inOutParametersInfo) const = 0;
            virtual void WriteParametersToGPUBuffer(SharedPtr<Buffer> buffer, const ShaderParametersInfo& parametersInfos, const void* pParameters) const = 0;

            virtual const Vector<Vector<ShaderParameterReflection::Type>>& GetCompatibleShaderParameterReflectionTypeMap() const = 0;
        };
    } // namespace gapi
} // namespace cube
