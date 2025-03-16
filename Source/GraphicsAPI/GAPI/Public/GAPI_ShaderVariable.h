#pragma once

#include "GAPIHeader.h"

#include "CubeString.h"

namespace cube
{
    namespace gapi
    {
        struct ShaderVariableInfo
        {
            const char* debugName = "Unknown";
        };

        class ShaderVariables
        {
        public:
            ShaderVariables() = default;
            virtual ~ShaderVariables() = default;
        };

        struct ShaderVariablesLayoutCreateInfo
        {
            const char* debugName = "Unknown";
        };

        class ShaderVariablesLayout
        {
        public:
            ShaderVariablesLayout() = default;
            virtual ~ShaderVariablesLayout() = default;
        };
    } // namespace gapi
} // namespace cube
