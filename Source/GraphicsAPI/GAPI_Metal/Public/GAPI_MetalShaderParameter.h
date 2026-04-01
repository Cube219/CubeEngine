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
            MetalShaderParameterHelper();
            virtual ~MetalShaderParameterHelper();

            void Initialize();
            void Shutdown();

            virtual void UpdateShaderParameterListInfo(ShaderParameterListInfo& inOutParameterListInfo) const override;
            virtual void WriteParametersToGPUBuffer(SharedPtr<Buffer> buffer, const ShaderParameterListInfo& parameterListInfo, const void* pParameterList) const override;

            virtual const Vector<Vector<ShaderParameterReflection::Type>>& GetCompatibleShaderParameterReflectionTypeMap() const override { return mCompatibleShaderParameterReflectionTypeMap; }

        private:
            void InitializeCompatibleShaderParameterReflectionTypeMap();

            Vector<Vector<ShaderParameterReflection::Type>> mCompatibleShaderParameterReflectionTypeMap;
        };
    } // namespace gapi
} // namespace cube
