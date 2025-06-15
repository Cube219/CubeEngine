#pragma once

#include "DX12Header.h"

#include "Blob.h"

struct IDxcCompiler3;
struct IDxcUtils;

namespace cube
{
    namespace gapi
    {
        struct ShaderCompileResult;
        struct ShaderCreateInfo;
    } // namespace gapi

    class DX12ShaderCompiler
    {
    public:
        DX12ShaderCompiler() = delete;
        ~DX12ShaderCompiler() = delete;

        static void Initialize();
        static void Shutdown();

        static Blob Compile(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult);

    private:
        static Blob CompileFromHLSL(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult);
        static Blob CompileFromDXIL(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult);
        static Blob CompileFromSlang(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult);

        static ComPtr<IDxcUtils> mUtils;
        static ComPtr<IDxcCompiler3> mCompiler;
    };
} // namespace cube
