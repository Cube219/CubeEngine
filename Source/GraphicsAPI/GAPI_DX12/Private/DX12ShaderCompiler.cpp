#include "DX12ShaderCompiler.h"

#include <d3dcompiler.h>

#include "DX12Device.h"
#include "GAPI_Shader.h"
#include "SlangHelper.h"

#ifndef CUBE_DX12_SLANG_TARGET_HLSL
#define CUBE_DX12_SLANG_TARGET_HLSL 0
#endif

namespace cube
{
    void DX12ShaderCompiler::Initialize()
    {
        SlangHelper::Initialize();
    }

    void DX12ShaderCompiler::Shutdown()
    {
        SlangHelper::Shutdown();
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
        case gapi::ShaderLanguage::Slang:
            return CompileFromSlang(createInfo, compileResult);
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
            compileResult.AddError(Format<FrameString>(CUBE_T("Failed to compile HLSL shader!\n\n{0}\n"), errorMessage));
            return nullptr;
        }
    }

    ID3DBlob* DX12ShaderCompiler::CompileFromDXIL(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult)
    {
        ID3DBlob* shaderBlob;
        D3DCreateBlob(createInfo.code.GetSize(), &shaderBlob);
        memcpy(shaderBlob->GetBufferPointer(), createInfo.code.GetData(), createInfo.code.GetSize());

        compileResult.isSuccess = true;
        return shaderBlob;
    }

    ID3DBlob* DX12ShaderCompiler::CompileFromSlang(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult)
    {
        const SlangCompileOptions compileOption = {
#if CUBE_DX12_SLANG_TARGET_HLSL
            .target = gapi::ShaderLanguage::HLSL
#else
            .target = gapi::ShaderLanguage::DXIL
#endif
        };
        Blob shader = SlangHelper::Compile(createInfo, compileOption, compileResult);

        if (compileResult.isSuccess)
        {
            // Compile the shader once again
            compileResult.isSuccess = false;

#if CUBE_DX12_SLANG_TARGET_HLSL
            gapi::ShaderCreateInfo hlslCreateInfo = createInfo;
            hlslCreateInfo.language = gapi::ShaderLanguage::HLSL;
            hlslCreateInfo.code = shader;

            return CompileFromHLSL(hlslCreateInfo, compileResult);
#else
            gapi::ShaderCreateInfo dxilCreateInfo = createInfo;
            dxilCreateInfo.language = gapi::ShaderLanguage::DXIL;
            dxilCreateInfo.code = shader;

            return CompileFromDXIL(dxilCreateInfo, compileResult);
#endif
        }

        return nullptr;
    }
} // namespace cube
