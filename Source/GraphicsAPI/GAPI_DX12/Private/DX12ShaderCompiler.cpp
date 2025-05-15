#include "DX12ShaderCompiler.h"

#include <d3dcompiler.h>

#include "DX12Device.h"
#include "GAPI_Shader.h"

namespace cube
{
    void DX12ShaderCompiler::Initialize()
    {
    }

    void DX12ShaderCompiler::Shutdown()
    {
    }

    ID3DBlob* DX12ShaderCompiler::Compile(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult)
    {
        compileResult.Reset();

        switch (createInfo.language)
        {
        case gapi::ShaderLanguage::HLSL:
            return CompileFromHLSL(createInfo, compileResult);
        case gapi::ShaderLanguage::DXIL:
            return CompileFromDXIL(createInfo, compileResult);
        default:
            compileResult.AddError(Format<FrameString>(CUBE_T("Not supported shader language: {0}"), (int)createInfo.language));
            return nullptr;
        }
    }

    ID3DBlob* DX12ShaderCompiler::CompileFromHLSL(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult)
    {
        const char* target = nullptr;
        switch (createInfo.type)
        {
        case gapi::ShaderType::Vertex:
            target = "vs_5_0";
            break;
        case gapi::ShaderType::Pixel:
            target = "ps_5_0";
            break;
        default:
            compileResult.AddError(Format<FrameString>(CUBE_T("Not supported shader type: {0}"), (int)createInfo.type));
            return nullptr;
        }

        int compilerFlags = D3DCOMPILE_DEBUG;

        ID3DBlob* shader;
        ComPtr<ID3DBlob> errorMessageBlob;
        HRESULT res = D3DCompile(createInfo.code.GetData(), createInfo.code.GetSize(), NULL, NULL, NULL, createInfo.entryPoint, target, compilerFlags, 0, &shader, &errorMessageBlob);
        if (res == S_OK)
        {
            compileResult.isSuccess = true;
            return shader;
        }
        else
        {
            const char* errorMessage = (const char*)errorMessageBlob->GetBufferPointer();
            compileResult.AddError(CUBE_T("Failed shader compilation!"));
            compileResult.AddError(String_Convert<String>(errorMessage), false);
            return nullptr;
        }
    }

    ID3DBlob* DX12ShaderCompiler::CompileFromDXIL(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult)
    {
        return nullptr;
    }
} // namespace cube
