#include "MetalShaderCompiler.h"

#include "Allocator/FrameAllocator.h"
#include "Checker.h"
#include "GAPI_Shader.h"
#include "Logger.h"
#include "MacOS/MacOSString.h"
#include "MetalDevice.h"
#include "SlangHelper.h"

#ifndef CUBE_DX12_SLANG_PRINT_METAL
#define CUBE_DX12_SLANG_PRINT_METAL 0
#endif

namespace cube
{
    MetalDevice* MetalShaderCompiler::mDevice;

    void MetalShaderCompiler::Initialize(MetalDevice* device)
    {
        mDevice = device;
        SlangHelper::Initialize();
    }

    void MetalShaderCompiler::Shutdown()
    {
        SlangHelper::Shutdown();
        mDevice = nullptr;
    }

    MetalShaderCompileResult MetalShaderCompiler::Compile(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult)
    {
        compileResult.Reset();

        switch (createInfo.language)
        {
        case gapi::ShaderLanguage::Metal:
            return CompileFromMetal(createInfo, compileResult);
        case gapi::ShaderLanguage::MetalLib:
            return CompileFromMetalLib(createInfo, compileResult);
        case gapi::ShaderLanguage::Slang:
            return CompileFromSlang(createInfo, compileResult);
        default:
            compileResult.AddError(Format<FrameString>(CUBE_T("Not supported shader language: {0}"), (int)createInfo.language));
            return {};
        }
    }

    MetalShaderCompileResult MetalShaderCompiler::CompileFromMetal(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult)
    { @autoreleasepool {
        FrameAnsiString source;
        {
            BlobView code = createInfo.shaderCodeInfos[0].code;

            source.resize(code.GetSize() + 1);
            memcpy(source.data(), code.GetData(), code.GetSize());
            source.back() = '\0';
        }

        NSString* code = String_Convert<NSString*>(source);
        NSError* error = nil;

        MetalShaderCompileResult result;
        result.library = [mDevice->GetMTLDevice() newLibraryWithSource:code options:nil error:&error];
        if (error)
        {
            compileResult.AddError(Format<FrameString>(CUBE_T("Failed to load the metal shader. ({0})"), [error localizedDescription]));
            return {};
        }

        NSString* entryPoint = String_Convert<NSString*>(createInfo.entryPoint);
        result.function = [result.library newFunctionWithName:entryPoint];

        return result;
    }}

    MetalShaderCompileResult MetalShaderCompiler::CompileFromMetalLib(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult)
    { @autoreleasepool {
        MetalShaderCompileResult result;
        NSError* error = nil;

        BlobView code = createInfo.shaderCodeInfos[0].code;

        dispatch_data_t data = dispatch_data_create(code.GetData(), code.GetSize(), nil, nil);
        result.library = [mDevice->GetMTLDevice() newLibraryWithData:data error:&error];
        dispatch_release(data);
        if (error)
        {
            compileResult.AddError(Format<FrameString>(CUBE_T("Failed to load the metal shader. ({0})"), [error localizedDescription]));
            return {};
        }

        NSString* entryPoint = String_Convert<NSString*>(createInfo.entryPoint);
        result.function = [result.library newFunctionWithName:entryPoint];

        return result;
    }}

    MetalShaderCompileResult MetalShaderCompiler::CompileFromSlang(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult)
    {
        const SlangCompileOptions compileOption = {
#if CUBE_DX12_SLANG_PRINT_METAL
            .target = gapi::ShaderLanguage::Metal
#else
            .target = gapi::ShaderLanguage::MetalLib
#endif
        };

        ShaderReflection reflection;
        Blob shader = SlangHelper::Compile(createInfo, compileOption, compileResult, &reflection);

        if (compileResult.isSuccess)
        {
#if CUBE_DX12_SLANG_PRINT_METAL
            FrameString pathList;
            for (auto& codeInfo : createInfo.shaderCodeInfos)
            {
                if (!pathList.empty())
                {
                    pathList.push_back(CUBE_T('\n'));
                }
                pathList += codeInfo.path;
            }
            FrameAnsiString shaderCode((const char*)shader.GetData(), shader.GetSize() / sizeof(char));
            CUBE_LOG(Info, DX12, "Compiled Metal code from\n\t{0}:\n{1}", pathList, shaderCode);
#endif
            // Compile the shader once again
            compileResult.isSuccess = false;

#if CUBE_DX12_SLANG_PRINT_METAL
            gapi::ShaderCreateInfo metalCreateInfo = createInfo;
            metalCreateInfo.language = gapi::ShaderLanguage::Metal;
            metalCreateInfo.shaderCodeInfos[0].code = shader;

            MetalShaderCompileResult result = CompileFromMetal(metalCreateInfo, compileResult);
#else
            gapi::ShaderCreateInfo metalLibCreateInfo = createInfo;
            metalLibCreateInfo.language = gapi::ShaderLanguage::MetalLib;
            metalLibCreateInfo.shaderCodeInfos[0].code = shader;

            MetalShaderCompileResult result = CompileFromMetalLib(metalLibCreateInfo, compileResult);
#endif
            // Use reflection from slang compiler first.
            result.reflection = std::move(reflection);
            return result;
        }

        return {};
    }
} // namespace cube
