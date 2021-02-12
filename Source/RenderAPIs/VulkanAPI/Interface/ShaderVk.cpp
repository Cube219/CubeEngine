#include "ShaderVk.h"

#include "../Tools/GLSLTool.h"
#include "../VulkanDebug.h"
#include "../VulkanDevice.h"
#include "Core/Assertion.h"

namespace cube
{
    namespace rapi
    {
        VkShaderStageFlagBits ConvertToVkShaderStageFlagBits(ShaderType shaderType)
        {
            switch(shaderType) {
                case ShaderType::Vertex:   return VK_SHADER_STAGE_VERTEX_BIT;
                case ShaderType::Pixel:    return VK_SHADER_STAGE_FRAGMENT_BIT;
                case ShaderType::Geometry: return VK_SHADER_STAGE_GEOMETRY_BIT;
                case ShaderType::Hull:     return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                case ShaderType::Domain:   return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                case ShaderType::Compute:  return VK_SHADER_STAGE_COMPUTE_BIT;
                default:
                    break;
            }

            ASSERTION_FAILED("Invalid ShaderType({}).", shaderType);
            return VK_SHADER_STAGE_ALL;
        }

        ShaderVk::ShaderVk(VulkanDevice& device, const ShaderCreateInfo& info) :
            Shader(info.debugName),
            mDevice(device)
        {
            VkResult res;

            switch(info.language) {
                case ShaderLanguage::GLSL:  LoadFromGLSL(info);  break;
                case ShaderLanguage::HLSL:  LoadFromHLSL(info);  break;
                case ShaderLanguage::SPIRV: LoadFromSPIRV(info); break;
                default: ASSERTION_FAILED("Unknown ShaderLanguage ({0}).", (Uint32)info.type); break;
            }

            VkShaderModuleCreateInfo moduleInfo;
            moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            moduleInfo.pNext = nullptr;
            moduleInfo.flags = 0;
            moduleInfo.codeSize = mSPIRV.size() * sizeof(Uint32);
            moduleInfo.pCode = mSPIRV.data();
            res = vkCreateShaderModule(device.GetHandle(), &moduleInfo, nullptr, &mShaderModule);
            CHECK_VK(res, "Failed to create VkShaderModule.");
            VULKAN_SET_OBJ_NAME(device.GetHandle(), mShaderModule, info.debugName);

            // Create pipeline shader stage info
            mShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            mShaderStageInfo.pNext = nullptr;
            mShaderStageInfo.flags = 0;
            mShaderStageInfo.pSpecializationInfo = nullptr;
            mShaderStageInfo.stage = ConvertToVkShaderStageFlagBits(info.type);
            mShaderStageInfo.pName = "main";
            mShaderStageInfo.module = mShaderModule;
        }

        ShaderVk::~ShaderVk()
        {
            vkDestroyShaderModule(mDevice.GetHandle(), mShaderModule, nullptr);
        }

        void ShaderVk::LoadFromGLSL(const ShaderCreateInfo& info)
        {
            GLSLTool::ToSPIRV(info.type, info.code, mSPIRV);
        }

        void ShaderVk::LoadFromHLSL(const ShaderCreateInfo & info)
        {
            ASSERTION_FAILED("Loading HLSL shader is not supported yet.");
        }

        void ShaderVk::LoadFromSPIRV(const ShaderCreateInfo & info)
        {
            ASSERTION_FAILED("Loading SPIR-V shader is not supported yet.");
        }
    } // namespace rapi
} // namespace cube
