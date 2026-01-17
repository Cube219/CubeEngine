#pragma once

#include "MetalHeader.h"

#include "ShaderParameterReflection.h"

namespace cube
{
    namespace gapi
    {
        struct ShaderCompileResult;
        struct ShaderCreateInfo;
    } // namespace gapi
    class MetalDevice;

    struct MetalShaderCompileResult
    {
        id<MTLLibrary> library = nil;
        id<MTLFunction> function = nil;

        ShaderReflection reflection;
    };

    class MetalShaderCompiler
    {
    public:
        MetalShaderCompiler() = delete;
        ~MetalShaderCompiler() = delete;

        static void Initialize(MetalDevice* device);
        static void Shutdown();

        static MetalShaderCompileResult Compile(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult);

    private:
        static MetalShaderCompileResult CompileFromMetal(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult);
        static MetalShaderCompileResult CompileFromMetalLib(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult);
        static MetalShaderCompileResult CompileFromSlang(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult);

        static MetalDevice* mDevice;
    };
} // namespace cube
