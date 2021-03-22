#pragma once

#include "../VulkanAPIHeader.h"

#include "RenderAPIs/RenderAPI/Interface/ShaderVariables.h"
#include "RenderAPIs/VulkanAPI/VulkanShaderVariableManager.h"

namespace cube
{
    namespace rapi
    {
        class ShaderVariablesVk : public ShaderVariables
        {
        public:
            ShaderVariablesVk(VulkanDevice& device, ShaderVariablesLayoutVk& layout, const char* debugName);
            virtual ~ShaderVariablesVk();

            virtual void UpdateVariable(Uint32 index, void* pData, Uint32 size) override;
            virtual void UpdateVariable(Uint32 index, SPtr<TextureView>& textureView) override;
            virtual void UpdateVariable(Uint32 index, SPtr<Sampler>& sampler) override;
            virtual void UpdateVariable(Uint32 index, SPtr<TextureView>& textureView, SPtr<Sampler>& sampler) override;

            void BIndVariables(VkCommandBuffer cmdBuf, VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, Uint32 layoutIndex);

        private:
            VulkanDevice& mDevice;
            ShaderVariablesLayoutVk& mLayout;

            VkDescriptorSet mDescriptorSet;
            Vector<ShaderVariableType> mVariableTypes;
            Vector<VulkanShaderVariableAllocation> mVariableAllocations;
        };

        class ShaderVariablesLayoutVk : public ShaderVariablesLayout
        {
        public:
            ShaderVariablesLayoutVk(VulkanDevice& device, const ShaderVariablesLayoutCreateInfo& info);
            virtual ~ShaderVariablesLayoutVk();

            virtual SPtr<ShaderVariables> CreateVariables() override;

            VkDescriptorSetLayout GetVkDescriptorSetLayout() const { return mDescriptorSetLayout; }
            const Vector<ShaderVariableInfo>& GetVariableInfos() const { return mVariableInfos; }

        private:
            VulkanDevice& mDevice;

            VkDescriptorSetLayout mDescriptorSetLayout;
            Vector<ShaderVariableInfo> mVariableInfos;
        };
    } // namespace rapi
} // namespace cube
