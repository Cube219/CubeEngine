#pragma once

#include "GAPIHeader.h"

#include "CubeString.h"

namespace cube
{
    namespace gapi
    {
        class Buffer;

        struct ShaderVariableInfo
        {
            const char* debugName = "Unknown";
        };

        struct ShaderVariableConstantBuffer
        {
            Uint32 index;
            SharedPtr<Buffer> buffer;
        };

        struct ShaderVariablesLayoutCreateInfo
        {
            Uint32 numShaderVariablesConstantBuffer;
            const ShaderVariableConstantBuffer* shaderVariablesConstantBuffer;

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
