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

            virtual void UpdateShaderParameterInfo(Vector<ShaderParameterInfo>& inOutParameterInfos, Uint32& outTotalBufferSize) const override;
            virtual void WriteParametersToGPUBuffer(SharedPtr<Buffer> buffer, const Vector<ShaderParameterInfo>& paramInfos, const void* pParameters) const override;

            virtual const Vector<Vector<ShaderParameterReflection::Type>>& GetCompatibleShaderParameterReflectionTypeMap() const override { return mCompatibleShaderParameterReflectionTypeMap; }

        private:
            void InitializeCompatibleShaderParameterReflectionTypeMap();

            Vector<Vector<ShaderParameterReflection::Type>> mCompatibleShaderParameterReflectionTypeMap;
        };
    } // namespace gapi
} // namespace cube
