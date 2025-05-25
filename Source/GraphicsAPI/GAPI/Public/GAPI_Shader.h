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
            SPIR_V,
            Metal,
            MetalLib,
            Slang
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
            case ShaderLanguage::Metal:
                return CUBE_T("Metal");
            case ShaderLanguage::MetalLib:
                return CUBE_T("MetalLib");
            case ShaderLanguage::Slang:
                return CUBE_T("Slang");
            default:
                return CUBE_T("Unknown");
            }
        }

        struct ShaderCreateInfo
        {
            ShaderType type;
            ShaderLanguage language;

            StringView fileName;
            StringView path;

            BlobView code;
            const char* entryPoint;

            const char* debugName = "Unknown";
        };

        class Shader
        {
        public:
            Shader() = default;
            virtual ~Shader() = default;

            bool IsValid() const { return mCreated; }
            StringView GetWarningMessage() const { return warningMessage; }
            StringView GetErrorMessage() const { return errorMessage; }

        protected:
            bool mCreated;
            String warningMessage;
            String errorMessage;
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
                    warning += Format(CUBE_T("Shader compile WARNING: {0}\n"), message);
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
                    error += Format(CUBE_T("Shader compile ERROR: {0}\n"), message);
                }
                else
                {
                    error += Format(CUBE_T("{0}\n"), message);
                }
            }
        };
    } // namespace gapi
} // namespace cube
