#include "ShaderVariablesVk.h"

#include "../VulkanDevice.h"
#include "SamplerVk.h"
#include "TextureViewVk.h"

#include "RenderAPIs/VulkanAPI/VulkanDebug.h"

namespace cube
{
    namespace rapi
    {
        ShaderVariablesVk::ShaderVariablesVk(VulkanDevice& device, ShaderVariablesLayoutVk& layout, const char* debugName) :
            mDevice(device),
            mLayout(layout)
        {
            auto& variableManager = mDevice.GetShaderVariableManager();

            mDescriptorSet = variableManager.AllocateDescriptorSet(layout.GetVkDescriptorSetLayout(), debugName);

            const Vector<ShaderVariableInfo>& variableInfos = layout.GetVariableInfos();

            // Write descriptor set to buffer (uniform, storage)
            FrameVector<VkWriteDescriptorSet> writeDescSets;
            FrameVector<VkDescriptorBufferInfo> writeBufferInfos;
            for(Uint64 i = 0; i < variableInfos.size(); ++i) {
                const ShaderVariableInfo& vInfo = variableInfos[i];

                VkWriteDescriptorSet write;
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.pNext = nullptr;
                write.dstSet = mDescriptorSet;
                write.dstBinding = SCast(Uint32)(i);
                write.dstArrayElement = 0;
                write.descriptorCount = 1;

                switch(vInfo.type) {
                    case ShaderVariableType::Constant:
                    {
                        VkDescriptorBufferInfo bufInfo;
                        bufInfo.buffer = variableManager.GetUniformDynamicBuffer();
                        bufInfo.offset = 0;
                        bufInfo.range = VK_WHOLE_SIZE;
                        writeBufferInfos.push_back(bufInfo);

                        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                        write.pBufferInfo = &writeBufferInfos.back();
                        write.pImageInfo = nullptr;
                        write.pTexelBufferView = nullptr;
                        break;
                    }

                    case ShaderVariableType::Storage:
                    {
                        VkDescriptorBufferInfo bufInfo;
                        bufInfo.buffer = variableManager.GetUniformDynamicBuffer();
                        bufInfo.offset = 0;
                        bufInfo.range = VK_WHOLE_SIZE;
                        writeBufferInfos.push_back(bufInfo);

                        write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                        write.pBufferInfo = &writeBufferInfos.back();
                        write.pImageInfo = nullptr;
                        write.pTexelBufferView = nullptr;
                        break;
                    }

                    case ShaderVariableType::ConstantTexture:
                    case ShaderVariableType::StorageTexture:
                    case ShaderVariableType::Sampler:
                    case ShaderVariableType::SamplerTexture:
                        break; // These types will be updated in ShaderVariablesVk::UpdateVariable

                    default:
                        ASSERTION_FAILED("Unsupported shader variable type in Vulkan ({})", (Uint32)vInfo.type);
                        continue;
                }

                writeDescSets.push_back(write);
            }

            vkUpdateDescriptorSets(mDevice.GetHandle(), SCast(Uint32)(writeDescSets.size()), writeDescSets.data(), 0, nullptr);

            // Allocate variables (uniform, storage)
            mVariableAllocations.resize(variableInfos.size());
            for(Uint64 i = 0; i < variableInfos.size(); ++i) {
                const ShaderVariableInfo& vInfo = variableInfos[i];

                switch(vInfo.type) {
                    case ShaderVariableType::Constant:
                    case ShaderVariableType::Storage:
                        mVariableAllocations[i] = variableManager.Allocate(vInfo.type, vInfo.size);
                        break;
                    default:
                        mVariableAllocations[i].pData = nullptr;
                        continue;
                }
            }

            mVariableTypes.resize(variableInfos.size());
            for(Uint32 i = 0; i < SCast(Uint32)(variableInfos.size()); ++i) {
                mVariableTypes[i] = variableInfos[i].type;
            }
        }

        ShaderVariablesVk::~ShaderVariablesVk()
        {
            auto& variableManager = mDevice.GetShaderVariableManager();

            for(auto& alloc : mVariableAllocations) {
                variableManager.Free(alloc);
            }
            variableManager.FreeDescriptorSet(mDescriptorSet);
        }

        void ShaderVariablesVk::UpdateVariable(Uint32 index, void* pData, Uint32 size)
        {
            ShaderVariableType vType = mVariableTypes[index];
            CHECK(vType == ShaderVariableType::Constant || vType == ShaderVariableType::Storage,
                "Only Constant/Storage type can be updated to byte data.");

            VulkanShaderVariableAllocation& alloc = mVariableAllocations[index];

            CHECK(alloc.size == size,
                "Data size doesn't match between allocated size({0}) and update size({1}).", alloc.size, size);

            memcpy(alloc.pData, pData, size);
        }

        void ShaderVariablesVk::UpdateVariable(Uint32 index, SPtr<TextureView>& textureView)
        {
            ShaderVariableType vType = mVariableTypes[index];
            CHECK(vType == ShaderVariableType::ConstantTexture || vType == ShaderVariableType::StorageTexture,
                "Only Constant/StorageTexture type can be updated to texture view.");

            VkWriteDescriptorSet write;
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.pNext = nullptr;
            write.dstSet = mDescriptorSet;
            write.dstBinding = index;
            write.dstArrayElement = 0;
            write.descriptorCount = 1;

            VkDescriptorImageInfo imgInfo;
            imgInfo.sampler = VK_NULL_HANDLE;
            imgInfo.imageView = DPCast(TextureViewVk)(textureView)->GetHandle();
            if(mVariableTypes[index] == ShaderVariableType::ConstantTexture) {
                imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            } else if(mVariableTypes[index] == ShaderVariableType::StorageTexture) {
                imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            }
            write.pImageInfo = &imgInfo;
            write.pBufferInfo = nullptr;
            write.pTexelBufferView = nullptr;

            vkUpdateDescriptorSets(mDevice.GetHandle(), 1, &write, 0, nullptr);
        }

        void ShaderVariablesVk::UpdateVariable(Uint32 index, SPtr<Sampler>& sampler)
        {
            ShaderVariableType vType = mVariableTypes[index];
            CHECK(vType == ShaderVariableType::Sampler,
                "Only Sampler type can be updated to sampler.");

            VkDescriptorImageInfo imgInfo;
            imgInfo.sampler = DPCast(SamplerVk)(sampler)->GetHandle();
            imgInfo.imageView = VK_NULL_HANDLE;
            imgInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            VkWriteDescriptorSet write;
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.pNext = nullptr;
            write.dstSet = mDescriptorSet;
            write.dstBinding = index;
            write.dstArrayElement = 0;
            write.descriptorCount = 1;
            write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
            write.pImageInfo = &imgInfo;
            write.pBufferInfo = nullptr;
            write.pTexelBufferView = nullptr;

            vkUpdateDescriptorSets(mDevice.GetHandle(), 1, &write, 0, nullptr);
        }

        void ShaderVariablesVk::UpdateVariable(Uint32 index, SPtr<TextureView>& textureView, SPtr<Sampler>& sampler)
        {
            ShaderVariableType vType = mVariableTypes[index];
            CHECK(vType == ShaderVariableType::SamplerTexture,
                "Only SamplerTexture type can be updated to texture view + sampler.");

            VkDescriptorImageInfo imgInfo;
            imgInfo.sampler = DPCast(SamplerVk)(sampler)->GetHandle();
            imgInfo.imageView = DPCast(TextureViewVk)(textureView)->GetHandle();
            imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkWriteDescriptorSet write;
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.pNext = nullptr;
            write.dstSet = mDescriptorSet;
            write.dstBinding = index;
            write.dstArrayElement = 0;
            write.descriptorCount = 1;
            write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write.pImageInfo = &imgInfo;
            write.pBufferInfo = nullptr;
            write.pTexelBufferView = nullptr;

            vkUpdateDescriptorSets(mDevice.GetHandle(), 1, &write, 0, nullptr);
        }

        void ShaderVariablesVk::BIndVariables(VkCommandBuffer cmdBuf, VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, Uint32 layoutIndex)
        {
            FrameVector<Uint32> dynamicOffsets;

            for(Uint64 i = 0; i < mVariableAllocations.size(); ++i) {
                auto& alloc = mVariableAllocations[i];
                dynamicOffsets.push_back(SCast(Uint32)(alloc.dynamicOffset));
            }

            vkCmdBindDescriptorSets(cmdBuf, bindPoint, pipelineLayout, layoutIndex, 1, &mDescriptorSet, 1, dynamicOffsets.data());
        }

        ShaderVariablesLayoutVk::ShaderVariablesLayoutVk(VulkanDevice& device, const ShaderVariablesLayoutCreateInfo& info) :
            mDevice(device)
        {
            VkResult res;

            VkDescriptorSetLayoutCreateInfo layoutInfo;
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.pNext = nullptr;
            layoutInfo.flags = 0;

            FrameVector<VkDescriptorSetLayoutBinding> bindings;
            for(Uint64 i = 0; i < info.variableInfos.size(); i++) {
                const ShaderVariableInfo& vInfo = info.variableInfos[i];

                VkDescriptorSetLayoutBinding binding;

                switch(vInfo.type) {
                    case ShaderVariableType::Constant:
                        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                        break;
                    case ShaderVariableType::Storage:
                        binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                        break;
                    case ShaderVariableType::ConstantTexture:
                        binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                        break;
                    case ShaderVariableType::StorageTexture: 
                        binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                        break;
                    case ShaderVariableType::Sampler: 
                        binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                        break;
                    case ShaderVariableType::SamplerTexture:
                        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        break;
                    default:
                        ASSERTION_FAILED("Unknown ShaderVariableType ()", vInfo.type);
                }

                binding.binding = SCast(Uint32)(i);
                binding.descriptorCount = vInfo.count;
                binding.stageFlags = VK_SHADER_STAGE_ALL;
                binding.pImmutableSamplers = nullptr;

                bindings.push_back(binding);
            }

            layoutInfo.bindingCount = SCast(Uint32)(bindings.size());
            layoutInfo.pBindings = bindings.data();

            res = vkCreateDescriptorSetLayout(mDevice.GetHandle(), &layoutInfo, nullptr, &mDescriptorSetLayout);
            CHECK_VK(res, "Failed to create VkDescriptorSetLayout");
            VULKAN_SET_OBJ_NAME(mDevice.GetHandle(), mDescriptorSetLayout, info.debugName);

            mVariableInfos.assign(info.variableInfos.begin(), info.variableInfos.end());
        }

        ShaderVariablesLayoutVk::~ShaderVariablesLayoutVk()
        {
            vkDestroyDescriptorSetLayout(mDevice.GetHandle(), mDescriptorSetLayout, nullptr);
        }

        SPtr<ShaderVariables> ShaderVariablesLayoutVk::CreateVariables()
        {
            return std::make_shared<ShaderVariablesVk>(mDevice, *this, "");
        }
    } // namespace rapi
} // namespace cube
