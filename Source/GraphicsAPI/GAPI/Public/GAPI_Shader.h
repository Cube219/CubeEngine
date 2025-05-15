#pragma once

#include "Blob.h"
#include "GAPIHeader.h"

#include "CubeString.h"
#include "Format.h"

namespace cube
{
    namespace gapi
    {
        enum class ShaderType
        {
            Vertex,
            Pixel,
            Compute
        };
        inline const Character* ShaderTypeToString(ShaderType shaderType)
        {
            switch (shaderType)
            {
            case ShaderType::Vertex:
                return CUBE_T("Vertex");
            case ShaderType::Pixel:
                return CUBE_T("Pixel");
            case ShaderType::Compute:
                return CUBE_T("Compute");
            default:
                return CUBE_T("Unknown");
            }
        }

        enum class ShaderLanguage
        {
            HLSL,
            GLSL,
            DXIL,
            SPIR_V
        };
        inline const Character* ShaderLanguageToString(ShaderLanguage shaderLanguage)
        {
            switch (shaderLanguage)
            {
            case ShaderLanguage::HLSL:
                return CUBE_T("HLSL");
            case ShaderLanguage::GLSL:
                return CUBE_T("GLSL");
            case ShaderLanguage::DXIL:
                return CUBE_T("DXIL");
            case ShaderLanguage::SPIR_V:
                return CUBE_T("SPIR_V");
            default:
                return CUBE_T("Unknown");
            }
        }

        struct ShaderCreateInfo
        {
            ShaderType type;
            ShaderLanguage language;

            BlobView code;
            const char* entryPoint;

            const char* debugName = "Unknown";
        };

        class Shader
        {
        public:
            Shader() = default;
            virtual ~Shader() = default;
        };

        struct ShaderCompileResult
        {
            bool isSuccess;
            String warning;
            String error;

            void Reset()
            {
                isSuccess = false;
                warning.clear();
                error.clear();
            }

            void AddWarning(StringView message, bool isBegin = true)
            {
                if (isBegin)
                {
                    warning += Format(CUBE_T("\tShader compile WARNING: {0}\n"), message);
                }
                else
                {
                    warning += Format(CUBE_T("{0}\n"), message);
                }
            }

            void AddError(StringView message, bool isBegin = true)
            {
                if (isBegin)
                {
                    warning += Format(CUBE_T("\tShader compile ERROR: {0}\n"), message);
                }
                else
                {
                    warning += Format(CUBE_T("{0}\n"), message);
                }
            }
        };
    } // namespace gapi
} // namespace cube
