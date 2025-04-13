#pragma once

#include "MetalHeader.h"

#include "GAPI_ShaderVariable.h"

namespace cube
{
    namespace gapi
    {
        class MetalShaderVariablesLayout : public ShaderVariablesLayout
        {
        public:
            MetalShaderVariablesLayout(const ShaderVariablesLayoutCreateInfo& info) {}
            virtual ~MetalShaderVariablesLayout() {}
        };
    } // namespace gapi
} // namespace cube
