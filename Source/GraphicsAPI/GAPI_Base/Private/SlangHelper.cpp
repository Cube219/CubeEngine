#include "SlangHelper.h"

#include <slang.h>
#include <slang-com-ptr.h>

#include "Allocator/FrameAllocator.h"
#include "Checker.h"
#include "Engine.h"
#include "GAPI_Shader.h"
#include "PathHelper.h"

using Slang::ComPtr;

namespace cube
{
    struct SlangHelperPrivate
    {
        static const Character* GetErrorCodeString(Int32 result);

        static void Initialize();
        static void Shutdown();

        static Blob Compile(const gapi::ShaderCreateInfo& info, const SlangCompileOptions& options, gapi::ShaderCompileResult& compileResult, ShaderReflection* pReflection);
        static void GetReflection(const gapi::ShaderCreateInfo& info, ComPtr<slang::IComponentType> program, ShaderReflection& outReflection);

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

    Blob SlangHelperPrivate::Compile(const gapi::ShaderCreateInfo& info, const SlangCompileOptions& options, gapi::ShaderCompileResult& compileResult, ShaderReflection* pReflection)
    {
        compileResult.Reset();
        ComPtr<slang::IBlob> diagnosticBlob;

        slang::TargetDesc targetDesc = {
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
        case gapi::ShaderLanguage::SPIRV:
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

        FrameVector<slang::PreprocessorMacroDesc> macros;
        macros.push_back({
            .name = "CUBE_SLANG_METAL",
            .value = (options.target == gapi::ShaderLanguage::Metal || options.target == gapi::ShaderLanguage::MetalLib) ? "1" : "0"
        });

        FrameVector<slang::CompilerOptionEntry> compilerOptions;
        if (options.withDebugSymbol)
        {
            compilerOptions.push_back({
                .name = slang::CompilerOptionName::DebugInformation,
                .value = {
                    .intValue0 = SLANG_DEBUG_INFO_LEVEL_MAXIMAL
                },
            });
            compilerOptions.push_back({
                .name = slang::CompilerOptionName::Optimization,
                .value = {
                    .intValue0 = SLANG_OPTIMIZATION_LEVEL_NONE
                },
            });
        }
        compilerOptions.push_back({
            .name = slang::CompilerOptionName::VulkanUseEntryPointName,
            .value = {
                .intValue0 = 1
            },
        });

        const char* shaderRootDirPath_CStr = mShaderSearchPath.c_str();
        const slang::SessionDesc sessionDesc = {
            .targets = &targetDesc,
            .targetCount = 1,
            .searchPaths = &shaderRootDirPath_CStr,
            .searchPathCount = 1,
            .preprocessorMacros = macros.data(),
            .preprocessorMacroCount = static_cast<Uint32>(macros.size()),
            .compilerOptionEntries = compilerOptions.data(),
            .compilerOptionEntryCount = static_cast<Uint32>(compilerOptions.size())
        };

        ComPtr<slang::ISession> session;
        CUBE_SLANG_CHECK(mGlobalSession->createSession(sessionDesc, session.writeRef()));

        FrameVector<slang::IComponentType*> componentTypes;

        // Load modules
        for (const auto& shaderCodeInfo : info.shaderCodeInfos)
        {
            FrameAnsiString name = String_Convert<FrameAnsiString>(PathHelper::GetFileNameFromPath(shaderCodeInfo.path));
            FrameAnsiString path = String_Convert<FrameAnsiString>(shaderCodeInfo.path);
            FrameAnsiString source;
            {
                source.resize(shaderCodeInfo.code.GetSize() + 1);
                memcpy(source.data(), shaderCodeInfo.code.GetData(), shaderCodeInfo.code.GetSize());
                source.back() = '\0';
            }

            slang::IModule* module = session->loadModuleFromSourceString(name.c_str(), path.c_str(), source.c_str(), diagnosticBlob.writeRef());
            if (module)
            {
                if (diagnosticBlob)
                {
                    compileResult.AddWarning(Format<FrameString>(CUBE_T("Warning about compiling Slang shader.\n\n{0}\n"), (const char*)diagnosticBlob->getBufferPointer()));
                }

                componentTypes.push_back(module);

                // Create entry point
                ComPtr<slang::IEntryPoint> entryPoint;
                module->findEntryPointByName(info.entryPoint.data(), entryPoint.writeRef());
                if (entryPoint)
                {
                    componentTypes.push_back(entryPoint);
                }
            }
            else
            {
                if (diagnosticBlob)
                {
                    compileResult.AddError(Format<FrameString>(CUBE_T("Failed to compile Slang shader!\n\n{0}\n"), (const char*)diagnosticBlob->getBufferPointer()));
                }
            }
        }

        if (!compileResult.error.empty())
        {
            return {};
        }

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

        // Link
        ComPtr<slang::IComponentType> linkedProgram;
        slangRes = composedProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
        if (slangRes != SLANG_OK)
        {
            compileResult.AddError(Format<FrameString>(CUBE_T("Failed to link the program! ({0})"), GetErrorCodeString(slangRes)));
            if (diagnosticBlob)
            {
                compileResult.AddError(Format<FrameString>(CUBE_T("\n{0}\n"), (const char*)diagnosticBlob->getBufferPointer()), false);
            }
            return {};
        }
        else if (diagnosticBlob)
        {
            compileResult.AddWarning(Format<FrameString>(CUBE_T("Warning about linking the program.\n\n{0}\n"), (const char*)diagnosticBlob->getBufferPointer()));
        }

        // Compile the code
        ComPtr<slang::IBlob> code;
        slangRes = linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnosticBlob.writeRef());
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

        if (pReflection)
        {
            GetReflection(info, linkedProgram, *pReflection);
        }

        return resBlob;
    }

    void SlangHelperPrivate::GetReflection(const gapi::ShaderCreateInfo& info, ComPtr<slang::IComponentType> program, ShaderReflection& outReflection)
    {
        using namespace slang;

        outReflection.blocks.clear();

        ProgramLayout* layout = program->getLayout();

        EntryPointReflection* entryPoint = layout->findEntryPointByName(info.entryPoint.data());
        SlangUInt threadGroupSize[3];
        entryPoint->getComputeThreadGroupSize(3, threadGroupSize);
        outReflection.threadGroupSizeX = threadGroupSize[0];
        outReflection.threadGroupSizeY = threadGroupSize[1];
        outReflection.threadGroupSizeZ = threadGroupSize[2];

        const Uint32 numParameterBlocks = layout->getParameterCount();
        for (Uint32 blockIndex = 0; blockIndex < numParameterBlocks; ++blockIndex)
        {
            VariableLayoutReflection* blockVariableLayout = layout->getParameterByIndex(blockIndex);
            TypeLayoutReflection* blockTypeLayout = blockVariableLayout->getTypeLayout();

            if (blockTypeLayout->getKind() == TypeReflection::Kind::ParameterBlock)
            {
                TypeLayoutReflection* parameterTypeLayout = blockTypeLayout->getElementTypeLayout();
                VariableLayoutReflection* parameterVariableLayout = blockTypeLayout->getElementVarLayout();

                const char* parameterTypeName = parameterTypeLayout->getName();
                Uint32 index = parameterVariableLayout->getBindingIndex();
                ShaderParameterBlockReflection& outBlockReflection = outReflection.blocks.emplace_back(parameterTypeName, index);
                
                if (parameterTypeLayout->getKind() == TypeReflection::Kind::Struct)
                {
                    Uint32 fieldCount = parameterTypeLayout->getFieldCount();
                    for (Uint32 fieldIndex = 0; fieldIndex < fieldCount; ++fieldIndex)
                    {
                        VariableLayoutReflection* fieldVariableLayout = parameterTypeLayout->getFieldByIndex(fieldIndex);
                        TypeLayoutReflection* fieldTypeLayout = fieldVariableLayout->getTypeLayout();
                        TypeReflection* fieldType = fieldVariableLayout->getType();

                        const char* fieldTypeName = fieldType->getName();
                        const char* fieldVariableName = fieldVariableLayout->getName();

                        auto AppendParmeterReflection = [&](ShaderParameterType type)
                        {
                            Uint32 offset = fieldVariableLayout->getOffset();
                            Uint32 size = fieldTypeLayout->getSize();
                            outBlockReflection.params.emplace_back(fieldVariableName, type, offset, size);
                        };

                        switch (fieldType->getKind())
                        {
                        case TypeReflection::Kind::Struct:
                        {
                            if (fieldTypeName == AnsiString("DescriptorHandle"))
                            {
                                // TODO
                                // AppendParmeterReflection(ShaderParameterType::Bindless);
                            }
                            else
                            {
                                CUBE_LOG(Warning, Slang, "Unsupported field type. Ignore it. (name: {0} / type: {1})", fieldVariableName, fieldTypeName);
                            }
                            break;
                        }
                        case TypeReflection::Kind::Matrix:
                        {
                            Uint32 numRow = fieldTypeLayout->getRowCount();
                            Uint32 numCol = fieldTypeLayout->getColumnCount();
                            if (numRow == 4 && numCol == 4)
                            {
                                AppendParmeterReflection(ShaderParameterType::Matrix);
                            }
                            else
                            {
                                CUBE_LOG(Warning, Slang, "Unsupported dimension in matrix. Only 4x4 is allowed. Ignore it. (name: {0} / row: {1} / col: {2})", fieldVariableName, numRow, numCol);
                            }
                            break;
                        }
                        case TypeReflection::Kind::Vector:
                        {
                            TypeReflection* vectorType = fieldType->getElementType();
                            TypeReflection::ScalarType vectorScalarType = vectorType->getScalarType();
                            if (vectorScalarType == TypeReflection::Float32)
                            {
                                Uint32 numElements = fieldType->getElementCount();
                                switch (numElements)
                                {
                                case 1:
                                    AppendParmeterReflection(ShaderParameterType::Float);
                                    break;
                                case 2:
                                    AppendParmeterReflection(ShaderParameterType::Float2);
                                    break;
                                case 3:
                                    AppendParmeterReflection(ShaderParameterType::Float3);
                                    break;
                                case 4:
                                    AppendParmeterReflection(ShaderParameterType::Float4);
                                    break;
                                default:
                                    NO_ENTRY_FORMAT("Invalid vector count: {0}", numElements);
                                    break;
                                }
                            }
                            else
                            {
                                CUBE_LOG(Warning, Slang, "Unsupported type in vector. Only float32 is allowed. Ignore it. (name: {0} / type: {1})", fieldVariableName, (int)vectorScalarType);
                            }
                            break;
                        }
                        case TypeReflection::Kind::Scalar:
                        {
                            TypeReflection::ScalarType scalarType = fieldTypeLayout->getScalarType();
                            switch (scalarType)
                            {
                            case TypeReflection::ScalarType::Bool:
                                AppendParmeterReflection(ShaderParameterType::Bool);
                                break;
                            case TypeReflection::ScalarType::Int32:
                                AppendParmeterReflection(ShaderParameterType::Int);
                                break;
                            case TypeReflection::ScalarType::Float32:
                                AppendParmeterReflection(ShaderParameterType::Float);
                                break;
                            default:
                                CUBE_LOG(Warning, Slang, "Unsupported type in scalar. Ignore it. (name: {0} / type: {1})", fieldVariableName, (int)scalarType);
                                break;
                            }
                            break;
                        }
                        default:
                            CUBE_LOG(Warning, Slang, "Unsupported field type. Ignore it. (name: {0} / type: {1})", fieldVariableName, fieldTypeName);
                            break;
                        }
                    }
                }
                else
                {
                    CUBE_LOG(Warning, Slang, "Only structure is allowed in ParameterBlock. Ignore it. (name: {0})", parameterTypeLayout->getName());
                }
            }
            else
            {
                CUBE_LOG(Warning, Slang, "Only ParameterBlock is allowed at the global scope. Ignore it. (name: {0})", blockVariableLayout->getName());
            }
        }
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

    Blob SlangHelper::Compile(const gapi::ShaderCreateInfo& info, const SlangCompileOptions& options, gapi::ShaderCompileResult& compileResult, ShaderReflection* pReflection)
    {
        return SlangHelperPrivate::Compile(info, options, compileResult, pReflection);
    }
} // namespace cube
