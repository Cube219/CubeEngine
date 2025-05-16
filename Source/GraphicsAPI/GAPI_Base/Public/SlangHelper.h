#pragma once

#include "GAPI_BaseHeader.h"

#include "GAPI_Shader.h"
#include "Allocator/FrameAllocator.h"

namespace cube
{
    struct SlangCompileOptions
    {
        gapi::ShaderLanguage target;
        const char* profile = "sm_6_0";
    };

    class SlangHelper
    {
    public:
        SlangHelper() = delete;
        ~SlangHelper() = delete;

        static void Initialize();
        static void Shutdown();

        static Blob Compile(const gapi::ShaderCreateInfo& info, const SlangCompileOptions& options, gapi::ShaderCompileResult& compileResult);
    };
} // namespace cube
