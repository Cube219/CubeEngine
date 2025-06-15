#include "DX12ShaderCompiler.h"

#include <dxcapi.h>

#include "DX12Device.h"
#include "GAPI_Shader.h"
#include "SlangHelper.h"
#include "Windows/WindowsString.h"

#ifndef CUBE_DX12_SLANG_TARGET_HLSL
#define CUBE_DX12_SLANG_TARGET_HLSL 0
#endif

namespace cube
{
    ComPtr<IDxcUtils> DX12ShaderCompiler::mUtils;
    ComPtr<IDxcCompiler3> DX12ShaderCompiler::mCompiler;

    void DX12ShaderCompiler::Initialize()
    {
        SlangHelper::Initialize();

        CHECK_HR(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&mUtils)));
        CHECK_HR(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&mCompiler)));
    }

    void DX12ShaderCompiler::Shutdown()
    {
        mCompiler = nullptr;
        mUtils = nullptr;

        SlangHelper::Shutdown();
    }

    Blob DX12ShaderCompiler::Compile(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult)
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
            return {};
        }
    }

    Blob DX12ShaderCompiler::CompileFromHLSL(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult)
    {
        using FrameWindowsString = TFrameString<WindowsCharacter>;

        FrameVector<const WindowsCharacter*> args;
        args.push_back(WINDOWS_T("-E"));
        FrameWindowsString entryPoint = String_Convert<FrameWindowsString>(createInfo.entryPoint);
        args.push_back(entryPoint.c_str());

        args.push_back(WINDOWS_T("-T"));
        switch (createInfo.type)
        {
        case gapi::ShaderType::Vertex:
            args.push_back(WINDOWS_T("vs_6_6"));
            break;
        case gapi::ShaderType::Pixel:
            args.push_back(WINDOWS_T("ps_6_6"));
            break;
        default:
            compileResult.AddError(Format<FrameString>(CUBE_T("Not supported shader type: {0}"), (int)createInfo.type));
            return {};
        }
        if (createInfo.withDebugSymbol)
        {
            args.push_back(WINDOWS_T("-Zi"));
            args.push_back(WINDOWS_T("-Qembed_debug"));
        }

        DxcBuffer sourceBuffer = {
            .Ptr = createInfo.code.GetData(),
            .Size = createInfo.code.GetSize(),
            .Encoding = DXC_CP_ACP
        };

        ComPtr<IDxcResult> dxcResult;
        CHECK_HR(mCompiler->Compile(&sourceBuffer, args.data(), args.size(), nullptr, IID_PPV_ARGS(&dxcResult)));

        HRESULT res;
        dxcResult->GetStatus(&res);
        if (res == S_OK)
        {
            compileResult.isSuccess = true;

            ComPtr<IDxcBlob> shader;
            dxcResult->GetResult(&shader);
            return Blob(shader->GetBufferPointer(), shader->GetBufferSize());
        }
        else
        {
            ComPtr<IDxcBlobEncoding> errorBlob;
            dxcResult->GetErrorBuffer(&errorBlob);
            if (errorBlob && errorBlob->GetBufferSize() > 0)
            {
                const char* errorMessage = (const char*)errorBlob->GetBufferPointer();
                compileResult.AddError(Format<FrameString>(CUBE_T("Failed to compile HLSL shader!\n\n{0}\n"), errorMessage));
            }
            else
            {
                compileResult.AddError(CUBE_T("Failed to compile HLSL shader!\n\nNo error message provided."));
            }
            return {};
        }
    }

    Blob DX12ShaderCompiler::CompileFromDXIL(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult)
    {
        ComPtr<IDxcBlobEncoding> shader;
        mUtils->CreateBlob(createInfo.code.GetData(), createInfo.code.GetSize(), DXC_CP_ACP, &shader);

        compileResult.isSuccess = true;
        return Blob(shader->GetBufferPointer(), shader->GetBufferSize());
    }

    Blob DX12ShaderCompiler::CompileFromSlang(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult)
    {
        SlangCompileOptions compileOption = {
#if CUBE_DX12_SLANG_TARGET_HLSL
            .target = gapi::ShaderLanguage::HLSL
#else
            .target = gapi::ShaderLanguage::DXIL
#endif
            ,
            .withDebugSymbol = createInfo.withDebugSymbol
        };
        if (compileOption.withDebugSymbol)
        {
            // DXIL does not support shader debugging on RenderDoc.
            compileOption.target = gapi::ShaderLanguage::HLSL;
        }

        Blob shader = SlangHelper::Compile(createInfo, compileOption, compileResult);

        if (compileResult.isSuccess)
        {
            // Compile the shader once again
            compileResult.isSuccess = false;

            if (compileOption.target == gapi::ShaderLanguage::HLSL)
            {
                gapi::ShaderCreateInfo hlslCreateInfo = createInfo;
                hlslCreateInfo.language = gapi::ShaderLanguage::HLSL;
                hlslCreateInfo.code = shader;

                return CompileFromHLSL(hlslCreateInfo, compileResult);
            }
            else if (compileOption.target == gapi::ShaderLanguage::DXIL)
            {
                gapi::ShaderCreateInfo dxilCreateInfo = createInfo;
                dxilCreateInfo.language = gapi::ShaderLanguage::DXIL;
                dxilCreateInfo.code = shader;

                return CompileFromDXIL(dxilCreateInfo, compileResult);
            }
        }

        return {};
    }
} // namespace cube
