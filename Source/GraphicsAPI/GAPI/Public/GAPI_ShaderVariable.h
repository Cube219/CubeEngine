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
            AnsiStringView debugName;
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

            StringView debugName;
        };

        class ShaderVariablesLayout
        {
        public:
            ShaderVariablesLayout() = default;
            virtual ~ShaderVariablesLayout() = default;
        };
    } // namespace gapi
} // namespace cube
