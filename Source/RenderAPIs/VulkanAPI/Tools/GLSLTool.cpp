#include "GLSLTool.h"

#include "Core/Assertion.h"

namespace cube
{
    namespace rapi
    {
        TBuiltInResource GLSLTool::mResources;

        void GLSLTool::Initialize()
        {
            glslang::InitializeProcess();

            InitResources();
        }

        void GLSLTool::Shutdown()
        {
            glslang::FinalizeProcess();
        }

        void GLSLTool::ToSPIRV(ShaderType type, const char* code, Vector<Uint32>& outSPIRV)
        {
            EShLanguage shLang;
            switch(type) {
                case ShaderType::Vertex:   shLang = EShLangVertex; break;
                case ShaderType::Pixel:    shLang = EShLangFragment; break;
                case ShaderType::Geometry: shLang = EShLangGeometry; break;
                case ShaderType::Hull:     shLang = EShLangTessControl; break;
                case ShaderType::Domain:   shLang = EShLangTessEvaluation; break;
                case ShaderType::Compute:  shLang = EShLangCompute; break;
                default: ASSERTION_FAILED("Unknown shader type ({0}).", (Uint32)type);
            }

            glslang::TShader shader(shLang);
            EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

            shader.setStrings(&code, 1);
            bool res = shader.parse(&mResources, 100, false, messages);
            CHECK(res, "Failed to parse glsl to spir-v. ({0} / {1})", shader.getInfoLog(), shader.getInfoDebugLog());

            glslang::TProgram program;
            program.addShader(&shader);
            res = program.link(messages);
            CHECK(res, "Failed to link program. ({0} / {1})", shader.getInfoLog(), shader.getInfoDebugLog());

            std::vector<Uint32> spirv_std;
            glslang::GlslangToSpv(*program.getIntermediate(shLang), spirv_std);

            outSPIRV.clear();
            outSPIRV.resize(spirv_std.size());
            outSPIRV.assign(spirv_std.begin(), spirv_std.end());
        }

        void GLSLTool::InitResources()
        {
            mResources.maxLights = 32;
            mResources.maxClipPlanes = 6;
            mResources.maxTextureUnits = 32;
            mResources.maxTextureCoords = 32;
            mResources.maxVertexAttribs = 64;
            mResources.maxVertexUniformComponents = 4096;
            mResources.maxVaryingFloats = 64;
            mResources.maxVertexTextureImageUnits = 32;
            mResources.maxCombinedTextureImageUnits = 80;
            mResources.maxTextureImageUnits = 32;
            mResources.maxFragmentUniformComponents = 4096;
            mResources.maxDrawBuffers = 32;
            mResources.maxVertexUniformVectors = 128;
            mResources.maxVaryingVectors = 8;
            mResources.maxFragmentUniformVectors = 16;
            mResources.maxVertexOutputVectors = 16;
            mResources.maxFragmentInputVectors = 15;
            mResources.minProgramTexelOffset = -8;
            mResources.maxProgramTexelOffset = 7;
            mResources.maxClipDistances = 8;
            mResources.maxComputeWorkGroupCountX = 65535;
            mResources.maxComputeWorkGroupCountY = 65535;
            mResources.maxComputeWorkGroupCountZ = 65535;
            mResources.maxComputeWorkGroupSizeX = 1024;
            mResources.maxComputeWorkGroupSizeY = 1024;
            mResources.maxComputeWorkGroupSizeZ = 64;
            mResources.maxComputeUniformComponents = 1024;
            mResources.maxComputeTextureImageUnits = 16;
            mResources.maxComputeImageUniforms = 8;
            mResources.maxComputeAtomicCounters = 8;
            mResources.maxComputeAtomicCounterBuffers = 1;
            mResources.maxVaryingComponents = 60;
            mResources.maxVertexOutputComponents = 64;
            mResources.maxGeometryInputComponents = 64;
            mResources.maxGeometryOutputComponents = 128;
            mResources.maxFragmentInputComponents = 128;
            mResources.maxImageUnits = 8;
            mResources.maxCombinedImageUnitsAndFragmentOutputs = 8;
            mResources.maxCombinedShaderOutputResources = 8;
            mResources.maxImageSamples = 0;
            mResources.maxVertexImageUniforms = 0;
            mResources.maxTessControlImageUniforms = 0;
            mResources.maxTessEvaluationImageUniforms = 0;
            mResources.maxGeometryImageUniforms = 0;
            mResources.maxFragmentImageUniforms = 8;
            mResources.maxCombinedImageUniforms = 8;
            mResources.maxGeometryTextureImageUnits = 16;
            mResources.maxGeometryOutputVertices = 256;
            mResources.maxGeometryTotalOutputComponents = 1024;
            mResources.maxGeometryUniformComponents = 1024;
            mResources.maxGeometryVaryingComponents = 64;
            mResources.maxTessControlInputComponents = 128;
            mResources.maxTessControlOutputComponents = 128;
            mResources.maxTessControlTextureImageUnits = 16;
            mResources.maxTessControlUniformComponents = 1024;
            mResources.maxTessControlTotalOutputComponents = 4096;
            mResources.maxTessEvaluationInputComponents = 128;
            mResources.maxTessEvaluationOutputComponents = 128;
            mResources.maxTessEvaluationTextureImageUnits = 16;
            mResources.maxTessEvaluationUniformComponents = 1024;
            mResources.maxTessPatchComponents = 120;
            mResources.maxPatchVertices = 32;
            mResources.maxTessGenLevel = 64;
            mResources.maxViewports = 16;
            mResources.maxVertexAtomicCounters = 0;
            mResources.maxTessControlAtomicCounters = 0;
            mResources.maxTessEvaluationAtomicCounters = 0;
            mResources.maxGeometryAtomicCounters = 0;
            mResources.maxFragmentAtomicCounters = 8;
            mResources.maxCombinedAtomicCounters = 8;
            mResources.maxAtomicCounterBindings = 1;
            mResources.maxVertexAtomicCounterBuffers = 0;
            mResources.maxTessControlAtomicCounterBuffers = 0;
            mResources.maxTessEvaluationAtomicCounterBuffers = 0;
            mResources.maxGeometryAtomicCounterBuffers = 0;
            mResources.maxFragmentAtomicCounterBuffers = 1;
            mResources.maxCombinedAtomicCounterBuffers = 1;
            mResources.maxAtomicCounterBufferSize = 16384;
            mResources.maxTransformFeedbackBuffers = 4;
            mResources.maxTransformFeedbackInterleavedComponents = 64;
            mResources.maxCullDistances = 8;
            mResources.maxCombinedClipAndCullDistances = 8;
            mResources.maxSamples = 4;
            mResources.limits.nonInductiveForLoops = 1;
            mResources.limits.whileLoops = 1;
            mResources.limits.doWhileLoops = 1;
            mResources.limits.generalUniformIndexing = 1;
            mResources.limits.generalAttributeMatrixVectorIndexing = 1;
            mResources.limits.generalVaryingIndexing = 1;
            mResources.limits.generalSamplerIndexing = 1;
            mResources.limits.generalVariableIndexing = 1;
            mResources.limits.generalConstantMatrixVectorIndexing = 1;
        }
    } // namespace rapi
} // namespace cube
