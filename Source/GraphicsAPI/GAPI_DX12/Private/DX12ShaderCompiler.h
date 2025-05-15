#pragma once

#include "DX12Header.h"

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

        static ID3DBlob* Compile(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult);

    private:
        static ID3DBlob* CompileFromHLSL(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult);
        static ID3DBlob* CompileFromDXIL(const gapi::ShaderCreateInfo& createInfo, gapi::ShaderCompileResult& compileResult);
    };
} // namespace cube
