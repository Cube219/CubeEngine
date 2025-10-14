#include "MetalShaderCompiler.h"

#include <spirv_msl.hpp>

#include "Allocator/FrameAllocator.h"
#include "Checker.h"
#include "GAPI_Shader.h"
#include "Logger.h"
#include "MacOS/MacOSString.h"
#include "MetalDevice.h"
#include "SlangHelper.h"

#ifndef CUBE_METAL_SLANG_TARGET_METAL_CODE
#define CUBE_METAL_SLANG_TARGET_METAL_CODE 0
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
        result.library = [mDevice->GetDevice() newLibraryWithSource:code options:nil error:&error];
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
        result.library = [mDevice->GetDevice() newLibraryWithData:data error:&error];
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
        // Currently compile to SPIRV first then convert to Metal using SPIRV-Cross
        // to support DescriptorHandle in slang.
        const SlangCompileOptions compileOption = {
            .target = gapi::ShaderLanguage::SPIRV,
            .withDebugSymbol = createInfo.withDebugSymbol
        };
        Blob spirvShader = SlangHelper::Compile(createInfo, compileOption, compileResult);

        if (compileResult.isSuccess)
        {
            compileResult.isSuccess = false;

            // Convert to Metal and compile
            Blob metalShaderCode;
            try
            {
                CHECK(spirvShader.GetSize() % sizeof(uint32_t) == 0);
                spirv_cross::CompilerMSL mslCompiler((const uint32_t*)spirvShader.GetData(), spirvShader.GetSize() / sizeof(uint32_t));

                spirv_cross::CompilerMSL::Options options;
                options.set_msl_version(2);
                options.argument_buffers_tier = spirv_cross::CompilerMSL::Options::ArgumentBuffersTier::Tier2;
                mslCompiler.set_msl_options(options);

                std::string result = mslCompiler.compile();
                metalShaderCode = Blob(result.data(), result.size());
            }
            catch (const std::runtime_error& e)
            {
                compileResult.AddError(Format<FrameString>(CUBE_T("Failed to compile SPIRV to Metal using SPIRV-Cross!\n{0}"), e.what()));

                return {};
            }

            gapi::ShaderCreateInfo metalCreateInfo = createInfo;
            metalCreateInfo.language = gapi::ShaderLanguage::Metal;
            metalCreateInfo.shaderCodeInfos[0].code = metalShaderCode;
            return CompileFromMetal(metalCreateInfo, compileResult);
        }

        return {};
    }

    MetalShaderCompileResult MetalShaderCompiler::CompileFromSlangDirectly(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult)
    {
        const SlangCompileOptions compileOption = {
#if CUBE_METAL_SLANG_TARGET_METAL_CODE
            .target = gapi::ShaderLanguage::Metal
#else
            .target = gapi::ShaderLanguage::MetalLib
#endif
        };
        Blob shader = SlangHelper::Compile(createInfo, compileOption, compileResult);

        if (compileResult.isSuccess)
        {
            // Compile the shader once again
            compileResult.isSuccess = false;

#if CUBE_METAL_SLANG_TARGET_METAL_CODE
            gapi::ShaderCreateInfo metalCreateInfo = createInfo;
            metalCreateInfo.language = gapi::ShaderLanguage::Metal;
            metalCreateInfo.shaderCodeInfos[0].code = shader;

            return CompileFromMetal(metalCreateInfo, compileResult);
#else
            gapi::ShaderCreateInfo metalLibCreateInfo = createInfo;
            metalLibCreateInfo.language = gapi::ShaderLanguage::MetalLib;
            metalLibCreateInfo.shaderCodeInfos[0].code = shader;

            return CompileFromMetalLib(metalLibCreateInfo, compileResult);
#endif
        }

        return {};
    }
} // namespace cube
