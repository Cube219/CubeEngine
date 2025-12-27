#pragma once

#include "DX12Header.h"

#include "Blob.h"
#include "ShaderParameterReflection.h"

struct IDxcCompiler3;
struct IDxcUtils;

namespace cube
{
    namespace gapi
    {
        struct ShaderCompileResult;
        struct ShaderCreateInfo;
    } // namespace gapi

    struct DX12ShaderCompilerResult
    {
        Blob shader;

        ShaderReflection reflection;
    };

    class DX12ShaderCompiler
    {
    public:
        DX12ShaderCompiler() = delete;
        ~DX12ShaderCompiler() = delete;

        static void Initialize();
        static void Shutdown();

        static DX12ShaderCompilerResult Compile(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult);

    private:
        static DX12ShaderCompilerResult CompileFromHLSL(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult);
        static DX12ShaderCompilerResult CompileFromDXIL(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult);
        static DX12ShaderCompilerResult CompileFromSlang(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult);

        static ComPtr<IDxcUtils> mUtils;
        static ComPtr<IDxcCompiler3> mCompiler;
    };
} // namespace cube
