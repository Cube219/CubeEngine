#pragma once

#include "../VulkanAPIHeader.h"

#include "RenderAPIs/RenderAPI/Interface/Shader.h"

namespace cube
{
    namespace rapi
    {
        class ShaderVk : public Shader
        {
        public:
            ShaderVk(VulkanDevice& device, const ShaderCreateInfo& info);
            virtual ~ShaderVk();

        private:
            void LoadFromGLSL(const ShaderCreateInfo& info);
            void LoadFromHLSL(const ShaderCreateInfo& info);
            void LoadFromSPIRV(const ShaderCreateInfo& info);

            VulkanDevice& mDevice;

            VkShaderModule mShaderModule;
            Vector<Uint32> mSPIRV;
        };
    } // namespace rapi
} // namespace cube
