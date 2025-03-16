#include "GAPI_DX12Shader.h"

#include <d3dcompiler.h>

#include "Checker.h"

#include "DX12Utility.h"

namespace cube
{
    namespace gapi
    {
        DX12Shader::DX12Shader(const ShaderCreateInfo& info)
        {
            switch (info.language)
            {
            case ShaderLanguage::HLSL:
                LoadFromHLSL(info);
                break;
            case ShaderLanguage::DXIL:
                // TODO
                // LoadFromDXIL(info);
                // break;
            default:
                CUBE_LOG(LogType::Error, DX12, "Not supported shader language: {}", (int)info.language);
                CHECK(false);
                break;
            }
        }

        DX12Shader::~DX12Shader()
        {
            if (mShader)
            {
                mShader->Release();
            }
        }

        void DX12Shader::LoadFromHLSL(const ShaderCreateInfo& info)
        {
            const char* target = nullptr;
            switch (info.type)
            {
            case ShaderType::Vertex:
                target = "vs_5_0";
                break;
            case ShaderType::Pixel:
                target = "ps_5_0";
                break;
            default:
                CUBE_LOG(LogType::Error, DX12, "Not supported shader type: {}", (int)info.type);
                CHECK(false);
                break;
            }

            int codeLen = strlen(info.code);
            int compilerFlags = 0;

            CHECK_HR(D3DCompile(info.code, codeLen, NULL, NULL, NULL, info.entryPoint, target, compilerFlags, 0, &mShader, NULL));
        }

        void DX12Shader::LoadFromDXIL(const ShaderCreateInfo& info)
        {
        }
    } // namespace gapi
} // namespace cube
