#include "SlangHelper.h"

#include <slang.h>
#include <slang-com-ptr.h>

#include "Allocator/FrameAllocator.h"
#include "Checker.h"
#include "Engine.h"
#include "GAPI_Shader.h"

using Slang::ComPtr;

namespace cube
{
    struct SlangHelperPrivate
    {
        static const Character* GetErrorCodeString(Int32 result);

        static void Initialize();
        static void Shutdown();
        static Blob Compile(const gapi::ShaderCreateInfo& info, const SlangCompileOptions& options, gapi::ShaderCompileResult& compileResult);

        static ComPtr<slang::IGlobalSession> mGlobalSession;
        static AnsiString mShaderSearchPath;
    };
    ComPtr<slang::IGlobalSession> SlangHelperPrivate::mGlobalSession;
    AnsiString SlangHelperPrivate::mShaderSearchPath;

#define CUBE_SLANG_CHECK(expr) \
    { \
        SlangResult res = expr; \
        if (res != SLANG_OK) \
        { \
            cube::Checker::ProcessFailedCheckFormatting(__FILE__, __LINE__, CUBE_T(#expr), CUBE_T("Slang check failed! ({0})"), SlangHelperPrivate::GetErrorCodeString(res)); \
        } \
    }

    const Character* SlangHelperPrivate::GetErrorCodeString(Int32 result)
    {
#define RETURN_IF_MATCH(value) \
    if (result == value) \
    { \
        return CUBE_T(#value); \
    } \

        RETURN_IF_MATCH(SLANG_FAIL);
        RETURN_IF_MATCH(SLANG_E_NOT_IMPLEMENTED);
        RETURN_IF_MATCH(SLANG_E_NO_INTERFACE);
        RETURN_IF_MATCH(SLANG_E_ABORT);
        RETURN_IF_MATCH(SLANG_E_INVALID_HANDLE);
        RETURN_IF_MATCH(SLANG_E_INVALID_ARG);
        RETURN_IF_MATCH(SLANG_E_OUT_OF_MEMORY);

        RETURN_IF_MATCH(SLANG_E_BUFFER_TOO_SMALL);
        RETURN_IF_MATCH(SLANG_E_UNINITIALIZED);
        RETURN_IF_MATCH(SLANG_E_PENDING);
        RETURN_IF_MATCH(SLANG_E_CANNOT_OPEN);
        RETURN_IF_MATCH(SLANG_E_NOT_FOUND);
        RETURN_IF_MATCH(SLANG_E_INTERNAL_FAIL);
        RETURN_IF_MATCH(SLANG_E_NOT_AVAILABLE);
        RETURN_IF_MATCH(SLANG_E_TIME_OUT);

        return CUBE_T("Unknown");

#undef RETURN_IF_MATCH
    }

#define CUBE_SLANG_CHECK_DIAGNOSTIC(blob) \
    { \
        if (blob) \
        { \
            cube::Checker::ProcessFailedCheckFormatting(__FILE__, __LINE__, CUBE_T("diagnosticBlob != nullptr"), CUBE_T("Slang check failed!\n\t({0})"), (const char*)blob->getBufferPointer()); \
        } \
    }

    void SlangHelperPrivate::Initialize()
    {
        CUBE_SLANG_CHECK(slang::createGlobalSession(mGlobalSession.writeRef()));

        mShaderSearchPath = Format<FrameAnsiString>(CUBE_ANSI_T("{0}/Shaders"), Engine::GetRootDirectoryPath());
    }

    void SlangHelperPrivate::Shutdown()
    {
        mGlobalSession = nullptr;
    }

    Blob SlangHelperPrivate::Compile(const gapi::ShaderCreateInfo& info, const SlangCompileOptions& options, gapi::ShaderCompileResult& compileResult)
    {
        compileResult.Reset();
        ComPtr<slang::IBlob> diagnosticBlob;

        slang::TargetDesc targetDesc = {
            .format = SLANG_HLSL,
            .profile = mGlobalSession->findProfile(options.profile)
        };
        switch (options.target)
        {
        case gapi::ShaderLanguage::HLSL:
            targetDesc.format = SLANG_HLSL;
            break;
        case gapi::ShaderLanguage::GLSL:
            targetDesc.format = SLANG_GLSL;
            break;
        case gapi::ShaderLanguage::DXIL:
            targetDesc.format = SLANG_DXIL;
            break;
        case gapi::ShaderLanguage::SPIR_V:
            targetDesc.format = SLANG_SPIRV;
            break;
        case gapi::ShaderLanguage::Metal:
            targetDesc.format = SLANG_METAL;
            break;
        case gapi::ShaderLanguage::MetalLib:
            targetDesc.format = SLANG_METAL_LIB;
            break;
        default:
            CHECK_FORMAT(false, "Not supported shader target type");
            break;
        }

        FrameVector<slang::CompilerOptionEntry> compilerOptions;

        const char* shaderRootDirPath_CStr = mShaderSearchPath.c_str();
        const slang::SessionDesc sessionDesc = {
            .targets = &targetDesc,
            .targetCount = 1,
            .searchPaths = &shaderRootDirPath_CStr,
            .searchPathCount = 1,
            .compilerOptionEntries = compilerOptions.data(),
            .compilerOptionEntryCount = static_cast<uint32_t>(compilerOptions.size())
        };

        ComPtr<slang::ISession> session;
        CUBE_SLANG_CHECK(mGlobalSession->createSession(sessionDesc, session.writeRef()));

        // Load module
        FrameAnsiString name = String_Convert<FrameAnsiString>(info.fileName);
        FrameAnsiString path = String_Convert<FrameAnsiString>(info.path);
        FrameAnsiString source;
        {
            source.resize(info.code.GetSize() + 1);
            memcpy(source.data(), info.code.GetData(), info.code.GetSize());
            source.back() = '\0';
        }
        slang::IModule* module = session->loadModuleFromSourceString(name.c_str(), path.c_str(), source.c_str(), diagnosticBlob.writeRef());
        if (diagnosticBlob)
        {
            // TODO: It also contains warning messages?
            compileResult.AddError(Format<FrameString>(CUBE_T("Failed to compile Slang shader!\n\n{0}\n"), (const char*)diagnosticBlob->getBufferPointer()));
        }
        if (!module)
        {
            return {};
        }

        // Create entry point
        ComPtr<slang::IEntryPoint> entryPoint;
        module->findEntryPointByName(info.entryPoint, entryPoint.writeRef());
        if (entryPoint == nullptr)
        {
            compileResult.AddError(Format<FrameString>(CUBE_T("Cannot find entry point: {0}"), info.entryPoint));
            return {};
        }

        FrameVector<slang::IComponentType*> componentTypes;
        componentTypes.push_back(module);
        componentTypes.push_back(entryPoint);

        // Check the composition
        ComPtr<slang::IComponentType> composedProgram;
        SlangResult slangRes = session->createCompositeComponentType(componentTypes.data(), componentTypes.size(), composedProgram.writeRef(), diagnosticBlob.writeRef());
        if (slangRes != SLANG_OK)
        {
            compileResult.AddError(Format<FrameString>(CUBE_T("Failed to composite the components! ({0})"), GetErrorCodeString(slangRes)));
            if (diagnosticBlob)
            {
                compileResult.AddError(Format<FrameString>(CUBE_T("\n{0}\n"), (const char*)diagnosticBlob->getBufferPointer()), false);
            }
            return {};
        }
        else if (diagnosticBlob)
        {
            compileResult.AddWarning(Format<FrameString>(CUBE_T("Warning about compositing the components.\n\n{0}\n"), (const char*)diagnosticBlob->getBufferPointer()));
        }

        ComPtr<slang::IBlob> code;
        slangRes = composedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnosticBlob.writeRef());
        if (slangRes != SLANG_OK)
        {
            compileResult.AddError(Format<FrameString>(CUBE_T("Failed to get the code! ({0})"), GetErrorCodeString(slangRes)));
            if (diagnosticBlob)
            {
                compileResult.AddError(Format<FrameString>(CUBE_T("\n{0}\n"), (const char*)diagnosticBlob->getBufferPointer()), false);
            }
            return {};
        }
        else if (diagnosticBlob)
        {
            compileResult.AddWarning(Format<FrameString>(CUBE_T("Warning about getting the code.\n\n{0}\n"), (const char*)diagnosticBlob->getBufferPointer()));
        }

        Blob resBlob((Byte*)code->getBufferPointer(), code->getBufferSize());

        compileResult.isSuccess = true;

        return resBlob;
    }

    // Interface class
    void SlangHelper::Initialize()
    {
        SlangHelperPrivate::Initialize();
    }

    void SlangHelper::Shutdown()
    {
        SlangHelperPrivate::Shutdown();
    }

    Blob SlangHelper::Compile(const gapi::ShaderCreateInfo& info, const SlangCompileOptions& options, gapi::ShaderCompileResult& compileResult)
    {
        return SlangHelperPrivate::Compile(info, options, compileResult);
    }
} // namespace cube
