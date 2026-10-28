#pragma once

#include "GAPIHeader.h"

#include "GAPI_ShaderReflection.h"

namespace cube
{
    struct ShaderParameterListInfo;

    namespace gapi
    {
        struct ShaderParameterListAllocationInfo
        {
            Vector<Uint32> offsets;
            Uint32 totalBufferSize;
        };

        class ShaderParameterHelper
        {
        public:
            ShaderParameterHelper() = default;
            virtual ~ShaderParameterHelper() = default;

            virtual void UpdateShaderParameterListInfo(ShaderParameterListInfo& inOutParameterListInfo) const = 0;
            virtual void WriteParameterListToBuffer(void* pBuffer, const ShaderParameterListInfo& parameterListInfo, const void* pParameterList) const = 0;

            virtual const Vector<Vector<ShaderParameterReflection::Type>>& GetCompatibleShaderParameterReflectionTypeMap() const = 0;
        };
    } // namespace gapi
} // namespace cube
