#pragma once

#include "../VulkanAPIHeader.h"

#include "RenderAPIs/RenderAPI/Interface/Shader.h"

namespace cube
{
    namespace rapi
    {
        VkShaderStageFlagBits ConvertToVkShaderStageFlagBits(ShaderType shaderType);

        class ShaderVk : public Shader
        {
        public:
            ShaderVk(VulkanDevice& device, const ShaderCreateInfo& info);
            virtual ~ShaderVk();

            VkPipelineShaderStageCreateInfo GetVkPipelineShaderStageCreateInfo() const { return mShaderStageInfo; }

        private:
            void LoadFromGLSL(const ShaderCreateInfo& info);
            void LoadFromHLSL(const ShaderCreateInfo& info);
            void LoadFromSPIRV(const ShaderCreateInfo& info);

            VulkanDevice& mDevice;

            VkShaderModule mShaderModule;
            VkPipelineShaderStageCreateInfo mShaderStageInfo;
            Vector<Uint32> mSPIRV;
        };
    } // namespace rapi
} // namespace cube
