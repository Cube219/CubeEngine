#pragma once

#include "GAPIHeader.h"

#include <source_location>

#include "Blob.h"
#include "CubeString.h"
#include "FileSystem.h"
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
            SPIRV,
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
            case ShaderLanguage::SPIRV:
                return CUBE_T("SPIRV");
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

            struct ShaderCodeInfo
            {
                const platform::FilePath& path;
                BlobView code;
            };
            ArrayView<ShaderCodeInfo> shaderCodeInfos;
            AnsiStringView entryPoint;

            bool withDebugSymbol;
            StringView debugName;
        };

        class Shader
        {
        public:
            Shader() = default;
            virtual ~Shader() = default;

            bool IsValid() const { return mCreated; }
            StringView GetWarningMessage() const { return mWarningMessage; }
            StringView GetErrorMessage() const { return mErrorMessage; }

            // TODO: Use reflection data
            Uint32 GetThreadGroupSizeX() const { return mThreadGroupSizeX; }
            Uint32 GetThreadGroupSizeY() const { return mThreadGroupSizeY; }
            Uint32 GetThreadGroupSizeZ() const { return mThreadGroupSizeZ; }

            const Vector<platform::FilePath>& GetDependencyFilePaths() const { return mDependencyFilePaths; }

        protected:
            bool mCreated;
            String mWarningMessage;
            String mErrorMessage;

            Uint32 mThreadGroupSizeX;
            Uint32 mThreadGroupSizeY;
            Uint32 mThreadGroupSizeZ;

            Vector<platform::FilePath> mDependencyFilePaths;
        };

        struct ShaderCompileResult
        {
            bool isSuccess;
            String warning;
            String error;
            Vector<platform::FilePath> dependencyFilePaths;

            void Reset()
            {
                isSuccess = false;
                warning.clear();
                error.clear();
                dependencyFilePaths.clear();
            }

            void AddWarning(StringView message, bool isBegin = true, const std::source_location location = std::source_location::current())
            {
                if (isBegin)
                {
                    warning += Format(CUBE_T("Shader compile WARNING ({0}:{1}): {2}\n"), location.file_name(), location.line(), message);
                }
                else
                {
                    warning += Format(CUBE_T("{0}\n"), message);
                }
            }

            void AddError(StringView message, bool isBegin = true, const std::source_location location = std::source_location::current())
            {
                if (isBegin)
                {
                    error += Format(CUBE_T("Shader compile ERROR: ({0}:{1}): {2}\n"), location.file_name(), location.line(), message);
                }
                else
                {
                    error += Format(CUBE_T("{0}\n"), message);
                }
            }
        };
    } // namespace gapi
} // namespace cube
